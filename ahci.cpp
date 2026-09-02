#include "ahci.h"

PortType AHCIDriver::check_port_type(HBA_PORT* port) {
    unsigned int ssts = port->ssts;
    unsigned char ipm = (ssts >> 8) & 0x0F;
    unsigned char det = ssts & 0x0F;

    if (det != 3 || ipm != 1) return PortType::None;

    switch (port->sig) {
        case 0x00000101: return PortType::SATA;
        case 0xEB140101: return PortType::SATAPI;
        case 0xC33C0101: return PortType::SEMB;
        case 0x96690101: return PortType::PM;
        default:         return PortType::None;
    }
}

void AHCIDriver::init(unsigned long long bar5_phys_addr) {
    hba_mem = reinterpret_cast<volatile HBA_MEM*>(bar5_phys_addr);
    hba_mem->ghc |= (1U << 31); // Bật bit GHC.AE

    unsigned int pi = hba_mem->pi;
    for (int i = 0; i < 32; i++) {
        if (pi & (1 << i)) {
            PortType type = check_port_type(const_cast<HBA_PORT*>(&hba_mem->ports[i]));
            if (type == PortType::SATA) {
                // Đã tìm thấy ổ SATA!
            }
        }
    }
}

int AHCIDriver::find_cmd_slot(HBA_PORT* port) {
    unsigned int slots = (port->sact | port->ci);
    for (int i = 0; i < 32; i++) {
        if ((slots & (1 << i)) == 0) return i;
    }
    return -1;
}

// ĐỌC SECTOR
bool AHCIDriver::read(HBA_PORT* port, unsigned long long lba, unsigned int count, unsigned short* buffer) {
    port->is = 0xFFFF;
    int slot = find_cmd_slot(port);
    if (slot == -1) return false;

    auto* cmd_header = reinterpret_cast<HBA_CMD_HEADER*>(port->clb);
    cmd_header += slot;
    cmd_header->cfl = sizeof(FIS_REG_H2D) / sizeof(unsigned int);
    cmd_header->w = 0; // 0 = Read
    cmd_header->prdtl = 1;

    auto* cmd_tbl = reinterpret_cast<HBA_CMD_TBL*>(cmd_header->ctba);
    cmd_tbl->prdt_entry[0].dba = static_cast<unsigned int>(reinterpret_cast<unsigned long long>(buffer));
    cmd_tbl->prdt_entry[0].dbc = (count * 512) - 1;
    cmd_tbl->prdt_entry[0].i = 1;

    auto* fis = reinterpret_cast<FIS_REG_H2D*>(&cmd_tbl->cfis);
    fis->fis_type = 0x27;
    fis->c = 1;
    fis->command = 0x25; // READ DMA EXT
    
    fis->lba0 = static_cast<unsigned char>(lba);
    fis->lba1 = static_cast<unsigned char>(lba >> 8);
    fis->lba2 = static_cast<unsigned char>(lba >> 16);
    fis->device = 1 << 6;
    fis->lba3 = static_cast<unsigned char>(lba >> 24);
    fis->lba4 = static_cast<unsigned char>(lba >> 32);
    fis->lba5 = static_cast<unsigned char>(lba >> 40);

    fis->countl = static_cast<unsigned char>(count);
    fis->counth = static_cast<unsigned char>(count >> 8);

    port->ci = (1 << slot); // Bắn lệnh

    while (true) {
        if ((port->ci & (1 << slot)) == 0) break;
        if (port->is & (1 << 30)) return false;
    }
    return true;
}

// GHI SECTOR
bool AHCIDriver::write(HBA_PORT* port, unsigned long long lba, unsigned int count, unsigned short* buffer) {
    port->is = 0xFFFF;
    int slot = find_cmd_slot(port);
    if (slot == -1) return false;

    auto* cmd_header = reinterpret_cast<HBA_CMD_HEADER*>(port->clb);
    cmd_header += slot;
    cmd_header->cfl = sizeof(FIS_REG_H2D) / sizeof(unsigned int);
    cmd_header->w = 1; // 1 = Write
    cmd_header->prdtl = 1;

    auto* cmd_tbl = reinterpret_cast<HBA_CMD_TBL*>(cmd_header->ctba);
    cmd_tbl->prdt_entry[0].dba = static_cast<unsigned int>(reinterpret_cast<unsigned long long>(buffer));
    cmd_tbl->prdt_entry[0].dbc = (count * 512) - 1;
    cmd_tbl->prdt_entry[0].i = 1;

    auto* fis = reinterpret_cast<FIS_REG_H2D*>(&cmd_tbl->cfis);
    fis->fis_type = 0x27;
    fis->c = 1;
    fis->command = 0x35; // WRITE DMA EXT
    
    fis->lba0 = static_cast<unsigned char>(lba);
    fis->lba1 = static_cast<unsigned char>(lba >> 8);
    fis->lba2 = static_cast<unsigned char>(lba >> 16);
    fis->device = 1 << 6;
    fis->lba3 = static_cast<unsigned char>(lba >> 24);
    fis->lba4 = static_cast<unsigned char>(lba >> 32);
    fis->lba5 = static_cast<unsigned char>(lba >> 40);

    fis->countl = static_cast<unsigned char>(count);
    fis->counth = static_cast<unsigned char>(count >> 8);

    port->ci = (1 << slot); // Bắn lệnh

    while (true) {
        if ((port->ci & (1 << slot)) == 0) break;
        if (port->is & (1 << 30)) return false;
    }
    return true;
}