#include "nvme.h"

void NVMeDriver::init(unsigned long long bar0_phys_addr) {
    regs = reinterpret_cast<volatile NVME_CONTROLLER_REGISTERS*>(bar0_phys_addr);
    
    // Vị trí thanh ghi Doorbells nằm ngay sau BAR0 + 0x1000
    doorbells = reinterpret_cast<volatile unsigned int*>(bar0_phys_addr + 0x1000);

    // 1. Tắt Controller trước khi cấu hình (Xóa bit Enable trong CC)
    regs->cc &= ~1U;

    // Chờ Controller về trạng thái không sẵn sàng (CSTS.RDY == 0)
    while (regs->csts & 1U);

    // 2. Thiết lập kích thước Admin Submission Queue (ASQ) và Completion Queue (ACQ)
    // Ví dụ: Queue size = 64 entry (0x3F cho 64 entries)
    regs->aqa = (0x3F << 16) | (0x3F); 

    // 3. Gán địa chỉ vật lý RAM cho ASQ và ACQ
    // (Trong Kernel thực tế bro cấp phát vùng nhớ RAM 4KB cho admin_sq và admin_cq)
    regs->asq = reinterpret_cast<unsigned long long>(admin_sq);
    regs->acq = reinterpret_cast<unsigned long long>(admin_cq);

    // 4. Bật Controller lên (Ghi bit CC.EN = 1, thiết lập Page Size = 4KB, I/O Command Set)
    regs->cc |= (0 << 20) | (0 << 16) | (0 << 11) | 1U;

    // Chờ Controller báo Ready (CSTS.RDY == 1)
    while ((regs->csts & 1U) == 0);

    // NVMe đã sẵn sàng nhận lệnh!
}

bool NVMeDriver::read(unsigned long long lba, unsigned int count, unsigned short* buffer) {
    // Đóng gói NVME_COMMAND cho lệnh Read I/O (Opcode 0x02)
    // Gán địa chỉ buffer vào prp1
    // Lắc chuông Doorbell (doorbells[0] = tail) để báo NVMe chip xử lý
    return true;
}

bool NVMeDriver::write(unsigned long long lba, unsigned int count, unsigned short* buffer) {
    // Đóng gói NVME_COMMAND cho lệnh Write I/O (Opcode 0x01)
    // Gán địa chỉ buffer vào prp1
    // Lắc chuông Doorbell để NVMe chip kéo dữ liệu từ RAM ghi xuống đĩa
    return true;
}