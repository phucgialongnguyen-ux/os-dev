#include "nvme.h"

// ============================================================================
// BỘ NHỚ CẤP PHÁT TĨNH ALIGNED 4KB CHO NVMe QUEUES
// NVMe Controller yêu cầu địa chỉ RAM của Submission/Completion Queue 
// BẮT BUỘC phải Aligned 4096-bytes (4KB Page Boundary)
// ============================================================================
alignas(4096) static NVME_COMMAND static_admin_sq[64];
alignas(4096) static NVME_COMPLETION static_admin_cq[64];

// ============================================================================
// HÀM: init
// MỤC ĐÍCH: Khởi tạo NVMe Controller qua BAR0 MMIO Address & Khởi tạo Admin Queue Pair
// ============================================================================
void NVMeDriver::init(unsigned long long bar0_phys_addr) {
    // Map thanh ghi Controller MMIO
    regs = reinterpret_cast<volatile NVME_CONTROLLER_REGISTERS*>(bar0_phys_addr);
    
    // Doorbells Register nằm ngay sau BAR0 ở offset 0x1000
    doorbells = reinterpret_cast<volatile unsigned int*>(bar0_phys_addr + 0x1000);

    // ------------------------------------------------------------------------
    // BƯỚC 1: TẮT CONTROLLER ĐỂ CẤU HÌNH (Reset CC.EN = 0)
    // ------------------------------------------------------------------------
    regs->cc &= ~1U;

    // Chờ cho Controller báo trạng thái đã Unready hoàn toàn (CSTS.RDY == 0)
    unsigned int timeout = 1000000;
    while ((regs->csts & 1U) && --timeout) { asm volatile("pause"); }

    // ------------------------------------------------------------------------
    // BƯỚC 2: THIẾT LẬP KÍCH THƯỚC ADMIN QUEUES (Thanh ghi AQA)
    // 16-bit cao = ACQ Size - 1, 16-bit thấp = ASQ Size - 1 (0x3F cho 64 entries)
    // ------------------------------------------------------------------------
    regs->aqa = (0x3F << 16) | (0x3F); 

    // ------------------------------------------------------------------------
    // BƯỚC 3: GÁN ĐỊA CHỈ VẬT LÝ VÙNG NHỚ RAM CỦA ADMIN QUEUES
    // Ghi địa chỉ 64-bit của ASQ va ACQ vào thanh ghi Controller
    // ------------------------------------------------------------------------
    admin_sq = static_admin_sq;
    admin_cq = static_admin_cq;
    regs->asq = reinterpret_cast<unsigned long long>(admin_sq);
    regs->acq = reinterpret_cast<unsigned long long>(admin_cq);

    // ------------------------------------------------------------------------
    // BƯỚC 4: BẬT CONTROLLER (Cấu hình thanh ghi CC - Controller Configuration)
    // Set CC.EN = 1, MPS = 0 (Host Page Size 4KB), CSS = 0 (NVM Command Set)
    // ------------------------------------------------------------------------
    regs->cc |= (0 << 20) | (0 << 16) | (0 << 11) | 1U;

    // Chờ Controller chuyển sang trạng thái Sẵn sàng hoàn toàn (CSTS.RDY == 1)
    timeout = 1000000;
    while (((regs->csts & 1U) == 0) && --timeout) { asm volatile("pause"); }
}

// ============================================================================
// HÀM: read
// MỤC ĐÍCH: Bắn NVMe Command Opcode 0x02 (Read) để đọc dữ liệu LBA trực tiếp vào RAM
// ============================================================================
bool NVMeDriver::read(unsigned long long lba, unsigned int count, unsigned short* buffer) {
    if (!admin_sq || !doorbells) return false;

    // 1. DỰNG NVME COMMAND STRUCT (64-Bytes Command)
    NVME_COMMAND cmd = {};
    cmd.opcode = 0x02; // Opcode 0x02: NVMe I/O Read
    cmd.nsid = 1;      // Namespace ID 1 (Mặc định cho đĩa NVMe chính)
    
    // PRP1 (Physical Region Page 1): Chứa địa chỉ RAM nhận dữ liệu
    cmd.prp1 = reinterpret_cast<unsigned long long>(buffer);
    
    // LBA 64-bit chia làm Dword 10 va Dword 11
    cmd.cdw10 = static_cast<unsigned int>(lba & 0xFFFFFFFF);
    cmd.cdw11 = static_cast<unsigned int>(lba >> 32);
    
    // CDW12: 16-bit thấp là số lượng Logical Blocks - 1 (0-based index)
    cmd.cdw12 = (count - 1) & 0xFFFF;

    // 2. NẠP LỆNH VÀO SUBMISSION QUEUE (SQ)
    admin_sq[0] = cmd;

    // 3. RUNG CHUÔNG DOORBELL (Ring Submission Queue Tail Doorbell)
    // Báo cho chip điều khiển NVMe biết có 1 câu lệnh mới ở Tail = 1
    doorbells[0] = 1; 

    // 4. POLING MẢNG COMPLETION QUEUE (CQ) ĐỂ CHỜ KẾT QUẢ
    unsigned int timeout = 10000000;
    while (timeout--) {
        if (admin_cq[0].status != 0) { // Khi NVMe chip xử lý xong, nó sẽ ghi Status != 0
            // Ring Completion Queue Head Doorbell (doorbells[1]) để giải phóng slot trong CQ
            doorbells[1] = 1; 
            return true; // Đọc thành công!
        }
        asm volatile("pause");
    }
    return false; // Timeout
}

// ============================================================================
// HÀM: write
// MỤC ĐÍCH: Bắn NVMe Command Opcode 0x01 (Write) để ghi dữ liệu từ RAM xuống LBA
// ============================================================================
bool NVMeDriver::write(unsigned long long lba, unsigned int count, unsigned short* buffer) {
    if (!admin_sq || !doorbells) return false;

    // 1. DỰNG NVME COMMAND STRUCT
    NVME_COMMAND cmd = {};
    cmd.opcode = 0x01; // Opcode 0x01: NVMe I/O Write
    cmd.nsid = 1;
    cmd.prp1 = reinterpret_cast<unsigned long long>(buffer); // Địa chỉ RAM chứa nguồn dữ liệu
    cmd.cdw10 = static_cast<unsigned int>(lba & 0xFFFFFFFF);
    cmd.cdw11 = static_cast<unsigned int>(lba >> 32);
    cmd.cdw12 = (count - 1) & 0xFFFF;

    // 2. ĐẶT LỆNH VÀO QUEUE & RUNG CHUÔNG DOORBELL
    admin_sq[0] = cmd;
    doorbells[0] = 1; // Tail Index = 1

    // 3. CHỜ PHẢN HỒI TỪ COMPLETION QUEUE
    unsigned int timeout = 10000000;
    while (timeout--) {
        if (admin_cq[0].status != 0) { // Chip NVMe đã kéo dữ liệu xong và báo Status
            doorbells[1] = 1; // Cập nhật Head Doorbell
            return true;
        }
        asm volatile("pause");
    }
    return false;
}