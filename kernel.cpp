__attribute__((section(".multiboot"))) const unsigned int boot_checksum[]{
		0x1BADB002,
		0x00,
		(unsigned int)-(0x1BADB002)
};

class Screen{
	private:
		volatile char* vga_display  = (volatile char*)0xB8000; 
		int curr_pos = 0;
		char curr_color = 0x07;
		volatile char* vga_graphic = (volatile char*)0xA0000;

	public:
		void Color(char color){
			curr_color = color;
		}
		bool CPUBootChecker(){
			for(int i = 0; i < 10; i++){
				if (1 + 1 != 2) return false;                    
				if (10121 * 12 != 121452) return false;               
				if (999 * 999 != 998001) return false;                
				if (123456 - 654321 != -530865) return false;         
				if (1024 * 1024 != 1048576) return false;             
				if (12345678 / 9 != 1371742) return false;            
				if (12345678 % 9 != 0) return false;                  
				if ((50 * 4) + (100 / 5) != 220) return false;        
				if ((999999 % 1234 != 303) && (123456 / 78 != 1582)) return false; 
				if (21474836 + 1 != 21474837) return false;
				}
				return true;
			}
		bool GPUBootChecker(){
			for(int i = 0; i < 100; i+=2){
				vga_graphic[i] = 0x00;
				vga_graphic[i + 1] = 0x07;
				if(vga_graphic[i] != 0x00 || vga_graphic[i + 1] != 0x07){
					return false;
				}
			}
			return true;
		}
		Screen& operator<<(const char* text){
			for(int i = 0 ; text[i] != '\0'; i++ ){
				if(text[i] == '\n' ){
					curr_pos = (curr_pos / 160 + 1) * 160;
					continue;
				}
				vga_display[curr_pos] = text[i];
				vga_display[curr_pos + 1] = curr_color;
				curr_pos += 2;
			}
			return *this;
		}
		void RestoreColor(){
			curr_color = 0x07;
		}
		void CleanUp(int Cleaner_limit = 4000){
			if(Cleaner_limit > 4000){
				*this << "Chuong Trinh Beo Qua";
				return;
			}
			for(int Cleaner = 0; Cleaner < Cleaner_limit; Cleaner++){
				vga_display[Cleaner] = ' ';
				vga_display[Cleaner + 1] = 0x07;
				 
			}
			curr_pos = 0;
			}
};



extern "C" void Kernel_main() {
    Screen print;
    Screen boot;
    boot.CPUBootChecker();
    boot.GPUBootChecker();

    print.Color(0x0A);
    print << "Hello World!!!!!! \n";
    print.RestoreColor();

    print.Color(0x0C);
    print << "\t [ WARNING : MYOS UNRESTRICTED CORE ACCESS ] \n";
    print << "Hey! WELCOME TO MYOS.\n";
    print << "Just a heads up before you jump in: This OS gives you 100% control over your\n";
    print << "hardware. No restrictions, no protective limits, no hand-holding.\n";
    print << "\n";
    print << "You can push the CPU, tweak the GPU, or mess with RAM directly if you want!\n";
    print << "However, if you end up frying your chip or blowing up a component, that's\n";
    print << "completely on you!! I am not responsible for any broken hardware or lost\n";
    print << "data.\n";
    print << "Remember you own the machine, you own everything!!!!\n";
    print.RestoreColor();

    while(1) {
        asm volatile("hlt");
    }
}