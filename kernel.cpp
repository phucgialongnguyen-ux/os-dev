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

	public:
		void Color(char color){
			curr_color = color;
		}
		Screen& operator<<(const char* text){
			for(int i = 0 ; text[i] != '\0'; i++ ){
				if(text[i] == '\\' ){
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

Screen print;

extern "C" void kernel_main(){
	print.Color(0x0A);
	print << "Hello World!\\";
	print.RestoreColor();
	while(1){
		asm volatile("htl");
	}
}
