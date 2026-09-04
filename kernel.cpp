#include "ahci.h"
#include "nvme.h"
__attribute__((section(".multiboot"))) const unsigned int boot_checksum[]{
        0x1BADB002,
        0x00,
        (unsigned int)-(0x1BADB002)
};

class Screen{
    private:
        volatile char* vga_display  = (volatile char*)0xB8000; 
        static inline int curr_pos = 0;
        static inline int curr_pos_limit = 0;
        char curr_color = 0x07;
        volatile char* vga_graphic = (volatile char*)0xA0000;

        static constexpr unsigned char ASCII_Control_Character_Table[] = {
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
            0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11,
            0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A,
            0x1B, 0x1C, 0x1D, 0x1E, 0x1F 
        };
        static constexpr unsigned char Printable_ASCII_Character_Table[] = {
            0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
            0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31,
            0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A,
            0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43,
            0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C,
            0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55,
            0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E,
            0x5F, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
            0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70,
            0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
            0x7A, 0x7B, 0x7C, 0x7D, 0x7E
        };
        static constexpr unsigned char Extended_ASCII[]{
            0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88,
            0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x91,
            0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A,
            0x9B, 0x9C, 0x9D, 0x9E, 0x9F, 0xA0, 0xA1, 0xA2, 0xA3,
            0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC,
            0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5,
            0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE,
            0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
            0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0,
            0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9,
            0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 0xE0, 0xE1, 0xE2,
            0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB,
            0xEC, 0xED, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4,
            0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD,
            0xFE, 0xFF
        };
    public:
        void Color(char color){
            curr_color = color;
        }

        inline bool CPUBootChecker(){
            for(int i = 0; i < 10; i++){
                if (1 + 1 != 2) return false;                    
                if (10121 * 12 != 121452) return false;               
                if (999 * 999 != 998001) return false;                
                if (123456 - 654321 != -530865) return false;         
                if (1024 * 1024 != 1048576) return false;             
                if (12345678 / 9 != 1371742) return false;            
                if (12345678 % 9 != 0) return false;                  
                if ((50 * 4) + (100 / 5) != 220) return false;        
                if ((999999 % 1234 != 723) && (123456 / 78 != 1582)) return false; 
                if (21474836 + 1 != 21474837) return false;
            }
            return true;
        }

        inline bool GPUBootChecker(){
            for(int i = 0; i < 30000; i += 2){
                vga_graphic[i] = 0x00;
                vga_graphic[i + 1] = 0x07;
                if(vga_graphic[i] != 0x00 || vga_graphic[i + 1] != 0x07){
                    *this << "GPUBootChecker Failed";
                    return false;
                }
            }
            return true;
        }

        static inline unsigned char input(unsigned short port){
            unsigned char ret;
            asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
            return ret;
        }

        static inline void output(unsigned short port, unsigned char val){
            asm volatile("outb %0, %1" :: "a"(val), "Nd"(port));
        }

        static inline unsigned short input16bit(unsigned short port){
            unsigned short ret;
            asm volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
            return ret;
        }
        static inline void output16bit(unsigned short port, unsigned short val){
            asm volatile("outw %0, %1" :: "a"(val), "Nd"(port));
        }
        static inline unsigned int input32bit(unsigned short port){
            unsigned int ret;
            asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
            return ret;
        }
        static inline void output32bit(unsigned short port, unsigned int val){
            asm volatile("outl %0, %1" :: "a"(val), "Nd"(port));
        }
        static inline void SysSpeaker(unsigned int freq){
            unsigned int base_freq = 1193180;
            unsigned int div = base_freq  / freq;
            output(0x43, 0xB6);
            output(0x42, (unsigned short)(div & 0xFF));
            output(0x42, (unsigned short)(div >> 8 & 0xFF));
            output(0x61, input(0x61) | 3);

        }
        static inline void SysSpeakerStop(){
            output(0x61, input(0x61) & 0xFC);
        }
        static inline void UpdateCursor(){
            static bool init = false; //i don't understand this shit so yeah i'll copy && paste then
            if(!init){
                output(0x3D4, 0x0A);
                output(0x3D5, (input(0x3D5) & 0xC0) | 13); 
                output(0x3D4, 0x0B);
                output(0x3D5, (input(0x3D5) & 0xE0) | 15); 
                init = true;
            }

            unsigned short index = curr_pos / 2;
            output(0x3D4, 0x0F);
            output(0x3D5, (unsigned char)(index & 0xFF));
            output(0x3D4, 0x0E);
            output(0x3D5, (unsigned char)((index >> 8) & 0xFF));

        }

        static constexpr unsigned char Scanner[128] = {
            0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
            '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
            0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
            0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
            '*', 0, ' '
        };
        static constexpr unsigned char Scanner_Shift[128] = {
            0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
            '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
            0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~',
            0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
            '*', 0, ' '
        };

        inline Screen& operator<<(const char* text){
            for(int i = 0 ; text[i] != '\0'; i++ ){
                if(text[i] == '\n' ){
                    curr_pos = (curr_pos / 160 + 1) * 160;
                    continue;
                }
                if(text[i] == '\b'){
                    if(curr_pos % 160 != 0){
                        if(curr_pos > curr_pos_limit){
                            curr_pos -= 2;
                            vga_display[curr_pos] = ' ';
                            vga_display[curr_pos + 1] = 0x07;
                            continue;
                        }
                    }
                }

                vga_display[curr_pos] = text[i];
                vga_display[curr_pos + 1] = curr_color;
                curr_pos += 2;
            }
            UpdateCursor();
            return *this;
        }
        inline Screen& operator<<(char text){
            char str[2] = {text, '\0'};
            *this << str;
            return *this;
        }
        inline Screen& operator<<(unsigned char text){
            return *this << (unsigned int)text;
        }
        inline Screen& operator<<(int text){
            char buffer[32];
            int i = 0;
            if(text == 0){
                buffer[i++] = '0';
            } else {
                int num = text;
                if(num < 0){
                    buffer[i++] = '-';
                    num = -num;
                }
                int start = i;
                while(num > 0){
                    buffer[i++] = (num % 10) + '0';
                    num /= 10;
                }
                for(int j = start, k = i - 1; j < k; j++, k--){
                    char temp = buffer[j];
                    buffer[j] = buffer[k];
                    buffer[k] = temp;
                }
            }
            buffer[i] = '\0';
            *this << buffer;
            return *this;
        }
        inline Screen& operator<<(unsigned int text){
            char buffer[32];
            int i = 0;
            if(text == 0){
                buffer[i++] = '0';
            } else {
                unsigned int num = text;
                int start = i;
                while(num > 0){
                    buffer[i++] = (num % 10) + '0';
                    num /= 10;
                }
                for(int j = start, k = i - 1; j < k; j++, k--){
                    char temp = buffer[j];
                    buffer[j] = buffer[k];
                    buffer[k] = temp;
                }
            }
            buffer[i] = '\0';
            *this << buffer;
            return *this;
        }
        inline Screen& operator<<(void* pr) {
            char buffer[32]; 
            int i = 0;
            unsigned long long num = (unsigned long long)pr; 
            if (num == 0) {
                buffer[i++] = '0';
            } else {
            int start = 0;
            while (num > 0) {
                unsigned long long digit = num % 16;
                if (digit < 10) {
                buffer[i++] = digit + '0';
                } else {
                    buffer[i++] = digit - 10 + 'A';
                }
                num /= 16;
            }
            for (int j = start, k = i - 1; j < k; j++, k--) {
                char temp = buffer[j];
                buffer[j] = buffer[k];
                buffer[k] = temp;
            }
        }
        buffer[i] = '\0';
        *this << buffer;
        return *this;
    }
        inline void RestoreColor(){
            curr_color = 0x07;
        }

        inline void CleanUp(int Cleaner_limit = 4000){
            if(Cleaner_limit > 4000){
                *this << "Chuong Trinh Beo Qua!";
                return;
            }
            for(int Cleaner = 0; Cleaner < Cleaner_limit; Cleaner += 2){
                vga_display[Cleaner] = ' ';
                vga_display[Cleaner + 1] = 0x07;
            }
            curr_pos = 0;
            UpdateCursor();
        }

        inline static unsigned char Keyboard_Driver(){
            if((input(0x64) & 1) == 0){
                return 0;
            }

            static bool is_Shift = false;
            static bool is_extended = false;
            unsigned char Scancode = input(0x60);

            if(Scancode == 0xE0){
                is_extended = true;
                return 0;
            }

            if(is_extended){
                is_extended = false;
                
                // this too
                if(Scancode == 0x48){}        // LÊN, khong con len duoc :)
                else if(Scancode == 0x50){} // XUỐNG, Khong con xuong duoc :)
                else if(Scancode == 0x4B){ if(curr_pos >= 2) curr_pos -= 2; }         // TRÁI
                else if(Scancode == 0x4D){ if(curr_pos + 2 < 4000) curr_pos += 2; }     // PHẢI

                UpdateCursor(); 
                return 0;
            }

            if(Scancode == 0x2A || Scancode == 0x36){ is_Shift = true; return 0; }
            if(Scancode == 0xAA || Scancode == 0xB6){ is_Shift = false; return 0; }

            if(Scancode < 0x80){
                char c = is_Shift ? Scanner_Shift[Scancode] : Scanner[Scancode];
                return c;
            }
            return 0;
        }
        void SomethingBeatifull(){
            Color(0x0B);
            *this << "yoursOS ";
            Color(0x07);
            *this << "user ";
            Color(0x0D);
            *this << "- ";
            Color(0x0A);
            *this << "$ ";
            RestoreColor();
            curr_pos_limit = curr_pos;

        }
        static inline void Primary_ATA_Controller(){
            while(((input(0x1F7) >> 7) & 1) == 1){}
            while(((input(0x1F7) >> 6 ) & 1) == 0){}
            output(0x1F6, 0xE0);

            output(0x1F2, 1);
            output(0x1F3, 0);
            output(0x1F4, 0);
            output(0x1F5, 0);
            output(0x1F7, 0x20);
        }
        static inline void Await_ata(){
        while(((input(0x1F7) >> 7) & 1) == 1){} 
        while(((input(0x1F7) >> 3) & 1) == 0){} 
}
       static inline void LBA28mod(unsigned int lba, unsigned char* buffer){
            while(((input(0x1F7) >> 7) & 1) == 1){} 
            while(((input(0x1F7) >> 6) & 1) == 0){}

            output(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
            output(0x1F2, 1);
            output(0x1F3, (lba & 0xFF));
            output(0x1F4, (lba >> 8) & 0xFF);
            output(0x1F5, (lba >> 16) & 0xFF);
            output(0x1F7, 0x20);

            Await_ata();

            unsigned short* ptr = (unsigned short*)buffer;
            for(int i = 0; i < 256; i++){
                ptr[i] = input16bit(0x1F0);
            }
        }
        inline void PCI_BUS(){
            for(unsigned int bus = 0; bus < 256; bus++){
                for(unsigned int device = 0; device < 32; device++){
                    for(unsigned int function = 0; function < 8; function++){
                        unsigned int offset = 0x00;
                        unsigned int address = (1U << 31) | (bus << 16) | (device << 11) | (function << 8) | offset;
                        output32bit(0xCF8, address);
                        unsigned int data = input32bit(0xCFC);
                        if(data == 0xFFFFFFFF){
                            if(function == 0){
                                break;
                            }
                            continue;
                        }
                        if(data != 0xFFFFFFFF){
                            unsigned short vendor_id = (unsigned short)(data & 0xFFFF);
                            unsigned short device_id = (unsigned short)((data >> 16) & 0xFFFF);
                            *this << "Vendor: 0x" << (void*)vendor_id << ", Device: 0x" << (void*)device_id << "\n";
                            if(vendor_id == 0x8086){
                                unsigned int adr_bar0 = (1U << 31) | (bus << 16) | (device << 11) | (function << 8) | 0x10;
                                output32bit(0xCF8, adr_bar0);
                                unsigned int bar0_raw = input32bit(0xCFC);
                                volatile unsigned int* mmio_base = (volatile unsigned int*)(bar0_raw & 0xFFFFFFF0);
                                *this << "BAR0 MMIO Base Address: 0x" << (void*)mmio_base << "\n";
                            }
                        }
                        if(function == 0){
                            unsigned int data2 = (1U << 31) | (bus << 16) | (device << 11) | 0x0C;
                            output32bit(0xCF8, data2);
                            unsigned int data3 = input32bit(0xCFC);
                            unsigned char header_type = (unsigned char)((data3 >> 16) & 0xFF);
                            if((header_type & 0x80) == 0){
                                break;
                            }
                        }
                    }
                }    
            }
        }
        inline void Reset(volatile unsigned int* mmio){
            unsigned int offset = 0x00;
            mmio[0x00 / 4] |= (1 << 26);
            for(volatile int i = 0; i < 2000 ; i++){}
        }
        inline void Clear_Interrupt(volatile unsigned int* mmio){
            unsigned int offset = 0x000D8;
            mmio[0x000D8 / 4] = 0xFFFFFFFF;
            
        }
        inline void Read_MAC(volatile unsigned int* mmio){
            unsigned int mac_low = mmio[0x05400 / 4];
            unsigned int mac_high = mmio[0x05404 / 4];
            unsigned char mac[6];
            mac[0] = mac_low & 0xFF;
        }
};
struct __attribute__((packed)) MBRPartitionEntry {
    unsigned char  boot_indicator; // I'll copy and paste this bullshit too anyway
    unsigned char  start_chs[3];  
    unsigned char  partition_type; 
    unsigned char  end_chs[3];     
    unsigned int   start_lba;     
    unsigned int   sector_count;    
};
struct __attribute__((packed)) Transmit_Descriptor{
    unsigned long long buffer_addr;
    unsigned short length;
    unsigned char cso;
    unsigned char cmd;
    unsigned char status;
    unsigned char css;
    unsigned short special;
};
struct __attribute__((packed)) Receive_Descriptor{
    unsigned long long buffer_addr;
    unsigned short length;
    unsigned short checksum;
    unsigned char status;
    unsigned char errors;
    unsigned short special;
};
AHCIDriver ahci_driver;
NVMeDriver nvme_driver;
extern "C" void kernel_main(unsigned long long pci_bar_addr, int drive_type) {
    Screen print;
    Screen boot;
    print.SomethingBeatifull();
    boot.CPUBootChecker();

    unsigned char buffer[512];
    Screen::LBA28mod(0, buffer);
    MBRPartitionEntry* entry1 = (MBRPartitionEntry*)&buffer[0x1BE];
    unsigned int all_sector = entry1->sector_count;
    unsigned int all_storange = all_sector / 2048;
    if (drive_type == 1) {
        ahci_driver.init(pci_bar_addr); 
    } 
    else if (drive_type == 2) {
        nvme_driver.init(pci_bar_addr); 
    } 
    else {  
        if (entry1->partition_type != 0) {
            print << "Partition 1 Found!\n";
            print << "Bootable: ";
            print << "Storage: " << all_storange << " MB\n";

            if (entry1->boot_indicator == 0x80) print << "Yes\n"; else print << "No\n";
        } else {
            print.Color(0x0C);
            print << "No Partition Found on Entry 1!\n";
            print << "Boot flag: " << (unsigned int)entry1->boot_indicator << "\n";
            print << "System ID: " << (unsigned int)entry1->partition_type << "\n";
            print << "Sector count: " << entry1->sector_count << "\n";
            print << "Magic Byte 1: " << (unsigned int)buffer[510] << "\n";
            print << "Magic Byte 2: " << (unsigned int)buffer[511] << "\n";
            print.RestoreColor();
            return;
        }
    }

    Screen::SysSpeaker(1000); // i tried this on virtualbox but i don't heard anything 
    for(volatile int i = 0; i < 20000000; i++){} 
    Screen::SysSpeakerStop();
 
    //print.Color(0x0A);
    //print << "Hello World!!!!!! \n";
    //print.RestoreColor();
    //rest in peace my friends "hello world", i won't forget you!! i promise.
    //print.Color(0x0A);
    //print << "Write something! \n";
    //print.RestoreColor();

    print.PCI_BUS();
    constexpr unsigned char cmd_storage[] = {"storage"};
    constexpr unsigned char cmd_help[] = {"help"};
    char cmd_buffer[32];
    int buffer_index = 0;// maybe.. in the future, i'll make 100 cmd

    Screen::UpdateCursor();
    while(1){
        char text = Screen::Keyboard_Driver();
        if(text != 0){
            if(text == '\n'){
                print << "\n";
                print.SomethingBeatifull(); //Sorry but no more 
            }  // char str[2] = {text, '\0'}; :( rip
            else{
                print << text;
            } 
        }
    }
}