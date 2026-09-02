#ifndef NVME_H
#define NVME_H

// Thanh ghi Controller Capabilities (CAP) - 64 bit từ BAR0
struct NVME_CAP {
    unsigned short mqes;        // Maximum Queue Entries Supported
    unsigned char  cqr : 1;     // Contiguous Queues Required
    unsigned char  ams : 2;     // Arbitration Mechanism Supported
    unsigned char  rsv0 : 5;
    unsigned char  to;          // Timeout (đơn vị 500ms)
    unsigned int   dstrd : 4;   // Doorbell Stride
    unsigned int   nssrs : 1;   // NVM Subsystem Reset Supported
    unsigned int   css : 8;     // Command Sets Supported
    unsigned int   bps : 1;     // Boot Partition Support
    unsigned int   rsv1 : 2;
    unsigned int   mpsmin : 4;  // Memory Page Size Minimum
    unsigned int   mpsmax : 4;  // Memory Page Size Maximum
    unsigned char  pmrs : 1;
    unsigned char  cmbs : 1;
    unsigned char  rsv2 : 6;
};

// Cấu trúc thanh ghi NVMe MMIO (Trỏ từ BAR0)
struct NVME_CONTROLLER_REGISTERS {
    unsigned long long cap;     // Controller Capabilities (0x00)
    unsigned int       vs;      // Version (0x08)
    unsigned int       intms;   // Interrupt Mask Set (0x0C)
    unsigned int       intmc;   // Interrupt Mask Clear (0x10)
    unsigned int       cc;      // Controller Configuration (0x14)
    unsigned int       rsv0;
    unsigned int       csts;    // Controller Status (0x1C)
    unsigned int       nssr;    // NVM Subsystem Reset (0x20)
    unsigned int       aqa;     // Admin Queue Attributes (0x24)
    unsigned long long asq;     // Admin Submission Queue Base Address (0x28)
    unsigned long long acq;     // Admin Completion Queue Base Address (0x30)
};

// Entry trong Submission Queue (64-byte lệnh gửi xuống NVMe)
struct NVME_COMMAND {
    unsigned char  opcode;      // Mã lệnh (0x02: Read, 0x01: Write)
    unsigned char  flags;
    unsigned short command_id;  // ID để định danh câu lệnh
    unsigned int   nsid;        // Namespace ID (Thường là 1)
    unsigned long long rsv0;
    unsigned long long mptr;    // Metadata Pointer
    unsigned long long prp1;    // Physical Region Page 1 (Địa chỉ RAM chứa buffer)
    unsigned long long prp2;    // Physical Region Page 2
    
    // Command Dwords (Tùy thuộc vào lệnh)
    unsigned int   cdw10;       // Với Read/Write: LBA Low 32-bit
    unsigned int   cdw11;       // Với Read/Write: LBA High 32-bit
    unsigned int   cdw12;       // Với Read/Write: Số lượng Sector (Blocks) - 1
    unsigned int   cdw13;
    unsigned int   cdw14;
    unsigned int   cdw15;
};

// Entry trong Completion Queue (16-byte phản hồi từ NVMe)
struct NVME_COMPLETION {
    unsigned int   command_specific;
    unsigned int   rsv0;
    unsigned short sq_head;    // Con trỏ Head của SQ
    unsigned short sq_id;      // ID của SQ
    unsigned short command_id; // ID lệnh khớp với NVME_COMMAND
    unsigned short status;     // Bit 0 = Phase Tag, Bit 1-15 = Mã lỗi/Trạng thái
};

class NVMeDriver {
public:
    void init(unsigned long long bar0_phys_addr);
    bool read(unsigned long long lba, unsigned int count, unsigned short* buffer);
    bool write(unsigned long long lba, unsigned int count, unsigned short* buffer);

private:
    volatile NVME_CONTROLLER_REGISTERS* regs = nullptr;
    volatile unsigned int* doorbells = nullptr;

    // Hàng đợi Admin Queues
    NVME_COMMAND*    admin_sq = nullptr;
    NVME_COMPLETION* admin_cq = nullptr;
    
    unsigned short admin_sq_tail = 0;
    unsigned short admin_cq_head = 0;
};

#endif