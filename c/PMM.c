//source: https://wiki.osdev.org/Page_Frame_Allocation
//and
//source: https://wiki.osdev.org/Memory_Allocation#The_Big_Picture
struct PMA{
    unsigned long long* memory_bit;
    unsigned long long* last_Allocated_Bit;
    unsigned int* buddy;
};
void Memory_Allocation(struct PMA* pma){
    unsigned long long index = 0;
    for(unsigned long long i = 0; i < 100; i++){
        if(((pma->memory_bit[index]) & (1ULL << i)) == 0){
            pma->memory_bit[index] |= (1ULL << i); 
            *(pma->last_Allocated_Bit) += 1;
        }
        else if(((pma->memory_bit[index]) & (1ULL << i)) != 0){
            unsigned long long* bitmap = pma->memory_bit;
            if(((bitmap[index]) & (1ULL << i)) == 0){
                //Comming Soon, I HATE School :(
                
            }        
        }    
    }
}
