#include "ahci.h"

// ============================================================================
// HÀM: check_port_type
// MỤC ĐÍCH: Kiểm tra xem một Port trên AHCI Controller đang kết nối với loại đĩa nào
// (SATA, SATAPI, SEMB, PM) bằng cách đọc thanh ghi SATA Status (ssts) và Signature (sig).
// ============================================================================
PortType AHCIDriver::check_port_type(HBA_PORT* port) {
    unsigned int ssts = port->ssts;
    
    // IPM (Interface Power Management): Bit 8-11 -> 0x01 nghia la Active state
    unsigned char ipm = (ssts >> 8) & 0x0F;
    
    // DET (Device Detection): Bit 0-3 -> 0x03 nghia la da ket noi va thiet lap physical link thành công
    unsigned char det = ssts & 0x0F;

    // Khong co thiet bi hoac link chua thiet lap xong
    if (det != 3 || ipm != 1) return PortType::None;

    // Doc thanh ghi Signature (port->sig) de phan biet loai thiet bi
    switch (port->sig) {
        case 0x00000101: return PortType::SATA;   // O cung HDD/SSD SATA thuong
        case 0xEB140101: return PortType::SATAPI; // O dia quang CD/DVD (dung lenh ATAPI)
        case 0xC33C0101: return PortType::SEMB;   // Enclosure Management Bridge
        case 0x96690101: return PortType::PM;     // Port Multiplier
        default:         return PortType::None;
    }
}

// ============================================================================
// HÀM: init
// MỤC ĐÍCH: Khởi tạo AHCI Controller thông qua địa chỉ MMIO BAR5 (Physical Address)
// ============================================================================
void AHCIDriver::init(unsigned long long bar5_phys_addr) {
    // Map BAR5 physical address sang con tro HBA_MEM (dung volatile de tranh GCC optimize)
    hba_mem = reinterpret_cast<volatile HBA_MEM*>(bar5_phys_addr);

    // Bat bit GHC.AE (AHCI Enable - Bit 31) de bao Controller hoat dong o che do AHCI
    hba_mem->ghc |= (1U << 31);

    // Doc bitmask Ports Implemented (PI) de biet nhung port nao dang co tren phan cung
    unsigned int pi = hba_mem->pi;
    for (int i = 0; i < 32; i++) {
        if (pi & (1 << i)) { // Neu Port i duoc ho tro
            PortType type = check_port_type(const_cast<HBA_PORT*>(&hba_mem->ports[i]));
            if (type == PortType::SATA) {
                // Da tim thay o SATA! Co the luu con tro &hba_mem->ports[i] vao mảng đĩa
            }
        }
    }
}

// ============================================================================
// HÀM: find_cmd_slot
// MỤC ĐÍCH: Tìm 1 slot rảnh (trống) trong tổng số 32 Command Slots của Port
// ============================================================================
int AHCIDriver::find_cmd_slot(HBA_PORT* port) {
    // Kombine 2 thanh ghi: SACT (SATA Active) va CI (Command Issue)
    // Bit nao bang 1 nghia la slot do dang duoc Controller xu ly
    unsigned int slots = (port->sact | port->ci);
    for (int i = 0; i < 32; i++) {
        if ((slots & (1 << i)) == 0) return i; // Tim thiet lap slot trong (bit = 0)
    }
    return -1; // Tất cả 32 slots đều đang bận
}

// ============================================================================
// HÀM: read
// MỤC ĐÍCH: Đọc N Sector (512 bytes/sector) từ đĩa qua DMA (Lệnh READ DMA EXT - 0x25)
// ============================================================================
bool AHCIDriver::read(HBA_PORT* port, unsigned long long lba, unsigned int count, unsigned short* buffer) {
    // Xoa cac co ngat cu tren Port
    port->is = 0xFFFF;
    
    // Tim Command Slot trong
    int slot = find_cmd_slot(port);
    if (slot == -1) return false;

    // 1. TÍNH ĐỊA CHỈ COMMAND HEADER 64-BIT (CLB + CLBU)
    // Shift clbu sang trai 32-bit va ghep voi clb
    unsigned long long clb_addr = (static_cast<unsigned long long>(port->clbu) << 32) | port->clb;
    auto* cmd_header = reinterpret_cast<HBA_CMD_HEADER*>(clb_addr);
    cmd_header += slot; // Tro toi Header cua slot vua tim duoc
    
    // CFL: Command FIS Length - do dai do bang so nguyen 32-bit (Dwords)
    cmd_header->cfl = sizeof(FIS_REG_H2D) / sizeof(unsigned int);
    cmd_header->w = 0;     // Bit W = 0: Thao tac DOC (Read) tu đĩa vào RAM
    cmd_header->prdtl = 1; // So luong PRDT entry duoc dung (o day dung 1 buffer)

    // 2. TÍNH ĐỊA CHỈ COMMAND TABLE 64-BIT (CTBA + CTBAU)
    unsigned long long ctba_addr = (static_cast<unsigned long long>(cmd_header->ctbau) << 32) | cmd_header->ctba;
    auto* cmd_tbl = reinterpret_cast<HBA_CMD_TBL*>(ctba_addr);

    // 3. THIẾT LẬP PRDT (Physical Region Descriptor Table) CHO DMA TRANSFER
    unsigned long long buf_addr = reinterpret_cast<unsigned long long>(buffer);
    cmd_tbl->prdt_entry[0].dba  = static_cast<unsigned int>(buf_addr & 0xFFFFFFFF); // 32-bit dia chi thap
    cmd_tbl->prdt_entry[0].dbau = static_cast<unsigned int>(buf_addr >> 32);         // 32-bit dia chi cao
    cmd_tbl->prdt_entry[0].dbc  = (count * 512) - 1;                                 // Byte count - 1
    cmd_tbl->prdt_entry[0].i    = 1;                                                 // Bat ngat khi DMA xong

    // 4. DỰNG COMMAND FIS (FIS Register Host-to-Device - 0x27)
    auto* fis = reinterpret_cast<FIS_REG_H2D*>(&cmd_tbl->cfis);
    fis->fis_type = 0x27; // Type 0x27: Register FIS - Host to Device
    fis->c = 1;        // Bit C = 1: Day la lenh moi phai gui toi thiet bi ngay
    fis->command = 0x25; // Opcode 0x25: READ DMA EXT (Doc LBA 48-bit)
    
    // Tach LBA 48-bit thanh 6 bytes rải vào các thanh ghi FIS
    fis->lba0 = static_cast<unsigned char>(lba);
    fis->lba1 = static_cast<unsigned char>(lba >> 8);
    fis->lba2 = static_cast<unsigned char>(lba >> 16);
    fis->device = 1 << 6; // Bit 6 = 1: Bat che do LBA Mode
    fis->lba3 = static_cast<unsigned char>(lba >> 24);
    fis->lba4 = static_cast<unsigned char>(lba >> 32);
    fis->lba5 = static_cast<unsigned char>(lba >> 40);

    // So luong Sector can doc (16-bit count)
    fis->countl = static_cast<unsigned char>(count);
    fis->counth = static_cast<unsigned char>(count >> 8);

    // 5. BẮN LỆNH TOÀN CỤC (Set Bit tuong ung trong Port Command Issue Register)
    port->ci = (1 << slot);

    // 6. CHỜ HOÀN TẤT VỚI TIMEOUT (Chống treo Kernel)
    unsigned int timeout = 10000000;
    while (timeout--) {
        // Neu bit tuong ung trong port->ci tu dong tra ve 0 nghia la Controller da xu ly xong!
        if ((port->ci & (1 << slot)) == 0) return true;
        
        // Bit 30 trong port->is = Task File Error (Đĩa bị lỗi/hỏng sector)
        if (port->is & (1 << 30)) return false;

        asm volatile("pause"); // Tiet kiem bus CPU trong khi kiem tra
    }
    return false; // Qua thoi gian ma dia khong phan hoi -> Timeout Error
}

// ============================================================================
// HÀM: write
// MỤC ĐÍCH: Ghi N Sector từ RAM xuống đĩa qua DMA (Lệnh WRITE DMA EXT - 0x35)
// ============================================================================
bool AHCIDriver::write(HBA_PORT* port, unsigned long long lba, unsigned int count, unsigned short* buffer) {
    port->is = 0xFFFF;
    int slot = find_cmd_slot(port);
    if (slot == -1) return false;

    // Lấy địa chỉ Command Header 64-bit chuẩn xác
    unsigned long long clb_addr = (static_cast<unsigned long long>(port->clbu) << 32) | port->clb;
    auto* cmd_header = reinterpret_cast<HBA_CMD_HEADER*>(clb_addr);
    cmd_header += slot;

    cmd_header->cfl = sizeof(FIS_REG_H2D) / sizeof(unsigned int);
    cmd_header->w = 1;     // Bit W = 1: Thao tac GHI (Write) tu RAM xuong đĩa
    cmd_header->prdtl = 1;

    // Lấy địa chỉ Command Table 64-bit
    unsigned long long ctba_addr = (static_cast<unsigned long long>(cmd_header->ctbau) << 32) | cmd_header->ctba;
    auto* cmd_tbl = reinterpret_cast<HBA_CMD_TBL*>(ctba_addr);

    // Tách buffer RAM 64-bit
    unsigned long long buf_addr = reinterpret_cast<unsigned long long>(buffer);
    cmd_tbl->prdt_entry[0].dba  = static_cast<unsigned int>(buf_addr & 0xFFFFFFFF);
    cmd_tbl->prdt_entry[0].dbau = static_cast<unsigned int>(buf_addr >> 32);
    cmd_tbl->prdt_entry[0].dbc  = (count * 512) - 1;
    cmd_tbl->prdt_entry[0].i    = 1;

    auto* fis = reinterpret_cast<FIS_REG_H2D*>(&cmd_tbl->cfis);
    fis->fis_type = 0x27;
    fis->c = 1;
    fis->command = 0x35; // Opcode 0x35: WRITE DMA EXT (Ghi LBA 48-bit)
    
    fis->lba0 = static_cast<unsigned char>(lba);
    fis->lba1 = static_cast<unsigned char>(lba >> 8);
    fis->lba2 = static_cast<unsigned char>(lba >> 16);
    fis->device = 1 << 6;
    fis->lba3 = static_cast<unsigned char>(lba >> 24);
    fis->lba4 = static_cast<unsigned char>(lba >> 32);
    fis->lba5 = static_cast<unsigned char>(lba >> 40);

    fis->countl = static_cast<unsigned char>(count);
    fis->counth = static_cast<unsigned char>(count >> 8);

    port->ci = (1 << slot); // Bắn lệnh xuống đĩa

    // Vòng lặp chờ kèm Timeout & Pause
    unsigned int timeout = 10000000;
    while (timeout--) {
        if ((port->ci & (1 << slot)) == 0) return true; // Ghi thành công!
        if (port->is & (1 << 30)) return false;         // Lỗi ghi đĩa
        asm volatile("pause");
    }
    return false;
}
