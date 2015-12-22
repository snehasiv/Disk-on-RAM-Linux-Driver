#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int errchk;
void menu(char dev[50])
{
	
	int opt=1;
	char pno[2];
	char cmd[100]={'\0'};
	char fsys[100]={'\0'};
		strcat(cmd,"dd if="); strcat(cmd,dev);
			if(!strcmp(dev,"/dev/rb"))
				strcat(cmd," of=dorb");
			else
				strcat(cmd," of=dorc");
		//strcat(cmd,"dd if="); strcat(cmd,dev); 
		system(cmd);
		strcpy(cmd,"\0");
		strcat(cmd,"fdisk -l ");
		strcat(cmd,dev);
		system(cmd);	
			
	while(opt)
	{
		printf("\nENTER (1) to write on disk");
		printf("\nENTER (2) to read on disk");
		printf("\nENTER (3) to format a partition and mount");
		printf("\nENTER (4) to unmount a partition");
		printf("\nENTER (5) for system log");
		printf("\nENTER 0 to exit");	
		scanf("%d",&opt);
		if(opt==0)
		{
			if(!strcmp(dev,"/dev/rb"))
				errchk = system("sudo rmmod dorb.ko");
			else
				errchk = system("sudo rmmod dorc.ko");
			if(errchk==-1)
			{
				
				printf("module cannot be removed... unmount filesystem first.\n");
			}
			else
			{
				
				printf("thank you for using RAMDisk\n");
				break;
			}
		}
		if(opt>=1 && opt <=4)
		{	
			//strcpy(cmd,"\0");strcat(cmd,"ls -l ");strcat(cmd,dev);system(cmd);
			printf("enter partition number from 1 to 7");
			scanf("%s",pno);
			if(!(pno[0]>='1' && pno[0]<='7'))
			{
				printf("\nincorrect partition number try again"); continue;
			}  
		}
		
		switch(opt)
		{
			case 5: 
					{
						system("dmesg | tail"); break;
					}
			case 1: 
					{		strcpy(cmd,"\0");
						
	  					strcat(cmd,"dd if=/dev/zero of=");
	  					strcat(cmd,dev);
 						strcat(cmd,pno);
 						strcat(cmd," count=1");
						system(cmd);
							strcpy(cmd,"\0");
							strcat(cmd,"sudo cat > "); //printf("CHECK: cmd=%s\n",cmd);
							strcat(cmd,dev); //printf("CHECK: cmd=%s\n",cmd);
							strcat(cmd,pno); //printf("CHECK: cmd=%s\n",cmd);
							system(cmd);  break;
					}
			case 2:
					{
							
							strcpy(cmd,"\0");
							strcat(cmd,"xxd "); //printf("CHECK: cmd=%s\n",cmd);
							strcat(cmd,dev); //printf("CHECK: cmd=%s\n",cmd);
							strcat(cmd,pno); //printf("CHECK: cmd=%s\n",cmd);
							strcat(cmd," | less "); system(cmd);  
							break;
					}	
			case 3:
					{
							strcpy(cmd,"\0");
							strcat(cmd,"sudo mkfs."); //printf("CHECK: cmd=%s\n",cmd);
							printf("\nEnter file system type for partition from \"vfat\" \"ext2\"");
							scanf("%s",fsys); 
							if(!strcmp(fsys,"vfat") || !strcmp(fsys,"ext2"))
							{
								strcat(cmd,fsys); //printf("CHECK: cmd=%s\n",cmd);
								strcat(cmd," "); //printf("CHECK: cmd=%s\n",cmd);
								strcat(cmd,dev); //printf("CHECK: cmd=%s\n",cmd);
								strcat(cmd,pno); //printf("CHECK: cmd=%s\n",cmd);									     
								system(cmd); 
								strcpy(cmd,"\0");
								strcat(cmd,"mkdir /mnt"); strcat(cmd,dev); strcat(cmd,pno);
								system(cmd); printf("CHECK: cmd=%s\n",cmd);
								strcpy(cmd,"\0");
								strcat(cmd,"sudo mount -t "); //printf("CHECK: cmd=%s\n",cmd);
								strcat(cmd,fsys); //printf("CHECK: cmd=%s\n",cmd);
								strcat(cmd," ");  strcat(cmd,dev);//printf("CHECK: cmd=%s\n",cmd);
								strcat(cmd,pno); //printf("CHECK: cmd=%s\n",cmd);
								strcat(cmd," /mnt"); //printf("CHECK: cmd=%s\n",cmd);
															
								strcat(cmd,dev); strcat(cmd,pno); 									printf("CHECK: cmd=%s\n",cmd);
								system(cmd); strcpy(cmd,"\0");
								strcat(cmd,"df"); //printf("CHECK: cmd=%s\n",cmd);
								system(cmd);
							}
							else
							{
								printf("\nincorrect file system type");
							}				
							break;
					}	
			case 4:
					{		 
							strcpy(cmd,"\0");
							strcat(cmd,"sudo umount ");
							strcat(cmd,dev);
							strcat(cmd,pno);
							system(cmd); strcpy(cmd,"\0");
								strcat(cmd,"df"); //printf("CHECK: cmd=%s\n",cmd);
								system(cmd);
							break;
					}
			default:
					{
							printf("\nincorrect entry.. try again"); }
		}
	}				
}				
int main()
  {
     int n;
	//system("gksudo");     	    
	 printf("\nChoose an option (1)Block Driver (2)Char Driver");
     scanf("%d",&n);
	     //system("cd /home/saloni/Downloads/DiskOnRAM");
         errchk = system("sudo make clean");
		if(errchk==-1)
		{
			printf("Module could not be compiled ... exiting");	
			exit(0);
		}	
	     errchk =system("sudo make");
		if(errchk==-1)
		{
			printf("Module could not be compiled ... exiting");
			exit(0);
		}
		if(n==1)
	      {	
	     errchk = system("sudo insmod dorb.ko");
		if(errchk==-1)
		{
			printf("Module could not be compiled ... exiting");
			exit(0);
		}
		//system("sudo dd if= dev/rb of= dor");  		
		menu("/dev/rb");
	}
     else
      	{
		errchk = system("sudo insmod dorc.ko");
		if(errchk==-1)
		{
			printf("Module could not be inserted ... exiting");
			exit(0);
		}
		//system("sudo dd if= /dev/rc of= dor");
		menu("/dev/rc");
		}
   return 0;  
  }   
