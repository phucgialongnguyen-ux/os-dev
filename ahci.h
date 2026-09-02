#ifndef AHCI_H
#define AHCI_H

enum class PortType {
    None = 0,
    SATA = 1,
    SEMB = 2,
    PM = 3,
    SATAPI = 4
};

// Cấu trúc PRDT - Quản lý vùng nhớ DMA
struct HBA_PRDT_ENTRY {
    unsigned int dba;       // Địa chỉ vật lý 32-bit thấp
    unsigned int dbau;      // Địa chỉ vật lý 32-bit cao
    unsigned int rsv0;
    unsigned int dbc : 22;  // Kích thước buffer - 1
    unsigned int rsv1 : 9;
    unsigned int i : 1;     // Cờ ngắt
};

// Command Table chứa Command FIS và PRDT
struct HBA_CMD_TBL {
    unsigned char cfis[64];
    unsigned char acmd[16];
    unsigned char rsv[48];
    HBA_PRDT_ENTRY prdt_entry[1];
};

// Command Header đại diện 1 câu lệnh trong Command List
struct HBA_CMD_HEADER {
    unsigned char cfl : 5;   // Độ dài FIS
    unsigned char a : 1;
    unsigned char w : 1;     // 1: Ghi, 0: Đọc
    unsigned char p : 1;
    unsigned char r : 1;
    unsigned char b : 1;
    unsigned char c : 1;
    unsigned char rsv0 : 1;
    unsigned char pmp : 4;
    unsigned short prdtl;    // Số lượng PRDT entry
    unsigned int prdbc;
    unsigned int ctba;      // Địa chỉ thấp của Command Table
    unsigned int ctbau;     // Địa chỉ cao của Command Table
    unsigned int rsv1[4];
};

// Thanh ghi của 1 Port AHCI
struct HBA_PORT {
    unsigned int clb;       // Command List Base Low
    unsigned int clbu;      // Command List Base High
    unsigned int fb;        // FIS Base Low
    unsigned int fbu;       // FIS Base High
    unsigned int is;        // Interrupt Status
    unsigned int ie;        // Interrupt Enable
    unsigned int cmd;       // Command and Status
    unsigned int rsv0;
    unsigned int tfd;       // Task File Data
    unsigned int sig;       // Signature (Nhận diện thiết bị)
    unsigned int ssts;      // SATA Status
    unsigned int sctl;      // SATA Control
    unsigned int serr;      // SATA Error
    unsigned int sact;      // SATA Active
    unsigned int ci;        // Command Issue (Bắn lệnh)
    unsigned int sntf;
    unsigned int fbs;
    unsigned int rsv1[11];
    unsigned int vendor[4];
};

// Cấu trúc MMIO tổng của AHCI Controller (BAR5)
struct HBA_MEM {
    unsigned int cap;       // Host Capabilities
    unsigned int ghc;       // Global Host Control (Bật AHCI Enable)
    unsigned int is;
    unsigned int pi;        // Ports Implemented
    unsigned int vs;
    unsigned int ccc_ctl;
    unsigned int ccc_pts;
    unsigned int em_loc;
    unsigned int em_ctl;
    unsigned int cap2;
    unsigned int bohc;
    unsigned char rsv[0xA0 - 0x2C];
    unsigned char vendor[0x100 - 0xA0];
    HBA_PORT ports[1];
};

// FIS Register H2D (Host to Device) gửi lệnh đến đĩa
struct FIS_REG_H2D {
    unsigned char fis_type; // 0x27
    unsigned char pmport : 4;
    unsigned char rsv0 : 3;
    unsigned char c : 1;     // 1: Lệnh mới
    unsigned char command;   // Lệnh READ (0x25) hoặc WRITE (0x35)
    unsigned char featurel;
    unsigned char lba0;      // LBA Bit 0-7
    unsigned char lba1;      // LBA Bit 8-15
    unsigned char lba2;      // LBA Bit 16-23
    unsigned char device;    // LBA Mode
    unsigned char lba3;      // LBA Bit 24-31
    unsigned char lba4;      // LBA Bit 32-39
    unsigned char lba5;      // LBA Bit 40-47
    unsigned char featureh;
    unsigned char countl;    // Số sector Low
    unsigned char counth;    // Số sector High
    unsigned char icc;
    unsigned char control;
    unsigned char rsv1[4];
};

class AHCIDriver {
public:
    void init(unsigned long long bar5_phys_addr);
    bool read(HBA_PORT* port, unsigned long long lba, unsigned int count, unsigned short* buffer);
    bool write(HBA_PORT* port, unsigned long long lba, unsigned int count, unsigned short* buffer);

private:
    volatile HBA_MEM* hba_mem = nullptr;
    PortType check_port_type(HBA_PORT* port);
    int find_cmd_slot(HBA_PORT* port);
};

#endif