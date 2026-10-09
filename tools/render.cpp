#include <cstdio>
#include <filesystem>
#include "../SquirrelPet.ino"
int main(int argc,char**argv) {
  if(argc!=2) return 1;
  const char* names[]={"meadow","picnic","play","sleep","digging","grooming"};
  const uint32_t base[]={0,0,0,0,8000,16000};
  for(int scene=0;scene<6;scene++) {
    std::string path=std::string(argv[1])+"/"+names[scene]+".rgb565";
    FILE* out=fopen(path.c_str(),"wb"); if(!out) return 2;
    sleeping=scene==3;
    action=scene==1?EATING:scene==2?PLAYING:IDLE;
    actionAt=0; food=80; joy=80; energy=90;
    for(int frame=0;frame<80;frame++) {
      render(base[scene]+frame*65);
      if(fwrite(screen.getBuffer(),2,240*135,out)!=240*135) return 3;
    }
    fclose(out);
  }
}
