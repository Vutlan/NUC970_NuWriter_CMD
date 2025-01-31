#include "common.h"
#include <string.h>
#include <unistd.h>  /* getopt */
#include <ctype.h>  /* isprint */
#include <dirent.h>
#include "config.h"

int check_strlen(const struct dirent *dir)
{
	if(strlen(dir->d_name)>5)
		return 1;
	else
		return 0;
}

void print_using()
{
	fprintf(stderr, "Download:\n");
	fprintf(stderr, "	./nuwriter -m sdram -d NUC972DF62Y.ini -a 0x8000 -w $IMAGE_PATH/kernel_970image\n");
	fprintf(stderr, "Downalod & run:\n");
	fprintf(stderr, "	./nuwriter -m sdram -d NUC972DF62Y.ini -a 0x8000 -w $IMAGE_PATH/kernel_970image -n\n");
	fprintf(stderr, "Download dtb(device tree):\n");
	fprintf(stderr, "	./nuwriter -m sdram -d NUC972DF62Y.ini -a 0x1E00000 -w $IMAGE_PATH/nuc970-evb.dtb\n");
	fprintf(stderr, "	./nuwriter -m sdram -d NUC972DF62Y.ini -a 0x8000 -w $IMAGE_PATH/kernel_970image -n -i 0x1E00000\n");
	fprintf(stderr, "Burn u-boot-spl.bin to NAND:\n");
	fprintf(stderr, "	./nuwriter -m nand -d NUC972DF62Y.ini -t uboot -a 0x200 -w $IMAGE_PATH/nand_uboot_spl.bin -v\n");
	fprintf(stderr, "Burn u-boot.bin to NAND:\n");
	fprintf(stderr, "	./nuwriter -m nand -d NUC972DF62Y.ini -t data -a 0x100000 -w $IMAGE_PATH/nand_uboot.bin -v\n");
	fprintf(stderr, "Burn env.txt to NAND:\n");
	fprintf(stderr, "	./nuwriter -m nand -d NUC972DF62Y.ini -t env -a 0x80000 -w $IMAGE_PATH/nand_env.txt -v\n");
	fprintf(stderr, "Burn linux to NAND:\n");
	fprintf(stderr, "	./nuwriter -m nand -d NUC972DF62Y.ini -t data -a 0x200000 -w $IMAGE_PATH/kernel_970uimage -v\n");
	fprintf(stderr, "Burn Pack image to NAND\n");
	fprintf(stderr, "	./nuwriter -m nand -d NUC972DF62Y.ini -t pack -w $IMAGE_PATH/nand_pack.bin\n");
	fprintf(stderr, "Erase NAND (chip erase):\n");
	fprintf(stderr, "	./nuwriter -m nand -d NUC972DF62Y.ini -e 0xffffffff\n");
	fprintf(stderr, "Erase NAND 10th block to 30th block:\n");
	fprintf(stderr, "	./nuwriter -m nand -d NUC972DF62Y.ini -a 10 -e 20\n");
	fprintf(stderr, "Read NAND 10th block to 30th block:\n");
	fprintf(stderr, "	./nuwriter -m nand -d NUC972DF62Y.ini -a 10 -e 1 -r $IMAGE_PATH/test.bin\n");
	fprintf(stderr, "Burn u-boot.bin to SPI:\n");
	fprintf(stderr, "	./nuwriter -m spi -d NUC972DF62Y.ini -t uboot -a 0xE00000 -w $IMAGE_PATH/spi_uboot.bin -v\n");
	fprintf(stderr, "Burn env.txt to SPI:\n");
	fprintf(stderr, "	./nuwriter -m spi -d NUC972DF62Y.ini -t env -a 0x80000 -w $IMAGE_PATH/spi_env.txt -v\n");
	fprintf(stderr, "Burn linux to SPI:\n");
	fprintf(stderr, "	./nuwriter -m spi -d NUC972DF62Y.ini -t data -a 0x200000 -w $IMAGE_PATH/kernel_970uimage -v\n");
	fprintf(stderr, "Burn Pack image to SPI\n");
	fprintf(stderr, "	./nuwriter -m spi -d NUC972DF62Y.ini -t pack -w $IMAGE_PATH/spi_pack.bin\n");
	fprintf(stderr, "Erase SPI (chip erase):\n");
	fprintf(stderr, "	./nuwriter -m spi -d NUC972DF62Y.ini -e 0xffffffff\n");
	fprintf(stderr, "Erase SPI 10th block to 30th block:\n");
	fprintf(stderr, "	./nuwriter -m spi -d NUC972DF62Y.ini -a 10 -e 20\n");
	fprintf(stderr, "Read SPI 10th block to 30th block:\n");
	fprintf(stderr, "	./nuwriter -m spi -d NUC972DF62Y.ini -a 10 -e 20 -r $IMAGE_PATH/test.bin\n");
	fprintf(stderr, "Burn u-boot.bin to eMMC:\n");
	fprintf(stderr, "	./nuwriter -m emmc -d NUC972DF62Y.ini -t uboot -a 0xE00000 -w $IMAGE_PATH/emmc_uboot.bin -v\n");
	fprintf(stderr, "Burn env.txt to eMMC:\n");
	fprintf(stderr, "	./nuwriter -m emmc -d NUC972DF62Y.ini -t env -a 0x80000 -w $IMAGE_PATH/emmc_env.txt -v\n");
	fprintf(stderr, "Burn linux to eMMC:\n");
	fprintf(stderr, "	./nuwriter -m emmc -d NUC972DF62Y.ini -t data -a 0x200000 -w $IMAGE_PATH/kernel_970uimage -v\n");
	fprintf(stderr, "Burn Pack image to eMMC\n");
	fprintf(stderr, "	./nuwriter -m emmc -d NUC972DF62Y.ini -t pack -w $IMAGE_PATH/emmc_pack.bin\n");
	fprintf(stderr, "Erase eMMC 10th block to 30th block:\n");
	fprintf(stderr, "	./nuwriter -m emmc -d NUC972DF62Y.ini -a 10 -e 20\n");
	fprintf(stderr, "Read eMMC 10th block to 30th block:\n");
	fprintf(stderr, "	./nuwriter -m emmc -d NUC972DF62Y.ini -a 10 -e 1 -r $IMAGE_PATH/test.bin\n");
}
int main(int argc, char **argv)
{
	/* Initial */
	char *path;
	int exitcode =0;

	type = -1;
	DDR_fileName[0]='\0';
	exe_addr=0xffffffff;
	dram_run = 0;
	erase_tag = 0;
	write_tag= 0;
	read_tag = 0;
	verify_tag = 0;
	dtb_tag = 0;

	usb_bus_cmdline = -1;
	usb_port_cmdline = -1;
	char execute_on_finish_buf[256];
	ssize_t execute_on_finish_len=0;

	int cmd_opt = 0;
	memset(execute_on_finish_buf,0, sizeof(execute_on_finish_buf));

	memcpy(Data_Path,argv[0],strlen(argv[0]));
	path=strrchr(Data_Path,'/');
	if(path==NULL) {
#ifndef _WIN32
		Data_Path[0]='.';
		Data_Path[1]='\0';
#else
		getcwd(Data_Path, 256);
#endif
	} else {
		*path='\0';
	}
#ifndef _WIN32
	sprintf(Data_Path,"%s%s",Data_Path,"/../share/nudata");
#else
	sprintf(Data_Path,"%s%s",Data_Path,"/nudata");
#endif
	//fprintf(stderr,"Data_Path=%s\n",Data_Path);
	//fprintf(stderr, "argc:%d\n", argc);
	while(1) {
		//fprintf(stderr, "proces index:%d\n", optind);
		cmd_opt = getopt(argc, argv, "a:d:e:i:nvhw:r:t:m:z::u:p:x:");

		/* End condition always first */
		if (cmd_opt == -1) {
			break;
		}

		/* Lets parse */
		switch (cmd_opt) {
		/* No args */
		case 'h':
			fprintf(stderr, "============================================\n");
			fprintf(stderr, "==   Nuvoton NuWriter Command Tool V%s   ==\n",PACKAGE_VERSION);
			fprintf(stderr, "============================================\n");

			fprintf(stderr, "NuWriter [Options] [File/Value]\n\n");
			fprintf(stderr, "-d [File]      Set DDR initial file\n");
#ifndef _WIN32
			fprintf(stderr, "-d show        Print supported DDR model\n");
#endif
			fprintf(stderr, "\n");
			fprintf(stderr, "-m sdram       Set SDRAM Mode\n");
			fprintf(stderr, "-m emmc        Set eMMC Mode\n");
			fprintf(stderr, "-m nand        Set NAND Mode\n");
			fprintf(stderr, "-m spi         Set SPI Mode\n");
			fprintf(stderr, "\n");
			fprintf(stderr, "-t data        Set DATA Type\n");
			fprintf(stderr, "-t env         Set Environemnt Type\n");
			fprintf(stderr, "-t uboot       Set uBoot Type\n");
			fprintf(stderr, "-t pack        Set PACK Type\n");
			fprintf(stderr, "\n");
			fprintf(stderr, "SDRAM parameters:\n");
			fprintf(stderr, "-a [value]     Set execute address\n");
			fprintf(stderr, "-w [File]      Write image file to SDRAM\n");
			fprintf(stderr, "-i [value]     device tree address\n");
			fprintf(stderr, "-n             Download & Run\n");
			fprintf(stderr, "\n");
			fprintf(stderr, "NAND/SPI/eMMC parameters:\n");
			fprintf(stderr, "-a [value]     Set start offset/execute address\n");
			fprintf(stderr, "               if erase, unit of block\n");
			fprintf(stderr, "-w [File]      Write image file to NAND\n");
			fprintf(stderr, "-r [File]      Read image file from NAND\n");
			fprintf(stderr, "-e [value]     Read/Erase length(unit of block)\n");
			fprintf(stderr, "-e 0xFFFFFFFF  Chip erase\n");
			fprintf(stderr, "-v             Verify image file after Write image \n");
			fprintf(stderr, "\n");
			fprintf(stderr, "Mass production parameters:\n");
			fprintf(stderr, "-u [value]     USB bus \n");
			fprintf(stderr, "-p [value]     USB port\n");
			fprintf(stderr, "-x [File]      Execute external program");
			fprintf(stderr, "\n============================================\n");
			return 0;
		case 'n':
			dram_run = 1;
			break;

		/* Single arg */
		case 'u':
			usb_bus_cmdline=strtoul(optarg,NULL,0);
			break;
		case 'p':
			usb_port_cmdline=strtoul(optarg,NULL,0);
			break;
		case 'x':
			execute_on_finish_len=snprintf(execute_on_finish_buf, sizeof(execute_on_finish_buf), "%s", optarg);
			break;

		case 'a':
			exe_addr=strtoul(optarg,NULL,0);
			break;
		case 'd': {
			char dir_path[256];
			sprintf(dir_path,"%s%s",Data_Path,"/sys_cfg");
#ifndef _WIN32
			if(strcasecmp(optarg,"show")==0) {
				struct dirent **namelist;
				int i, total;
				total = scandir(dir_path,&namelist,check_strlen,0);
				for(i=0; i<total; i++)
					fprintf(stderr,"%s\n",namelist[i]->d_name);
				return 0;
			} else {
				strcpy(DDR_fileName,optarg);
				//fprintf(stderr,"DDR_fileName=%s\n",DDR_fileName);
			}
#else
			strcpy(DDR_fileName,optarg);
#endif
		}
		break;
		case 'i':
			dtb_tag=1;
			dtb_addr=strtoul(optarg,NULL,0);
			break;
		case 'w':
			write_tag=1;
			strcpy(write_file,optarg);
			break;
		case 'r':
			read_tag=1;
			strcpy(read_file,optarg);
			break;
		case 'v':
			verify_tag=1;
			break;
		case 'm':
			if(strcasecmp(optarg,"sdram")==0) {
				//fprintf(stderr,"SDRAM mode\n");
				mode=SDRAM_M;
			} else if(strcasecmp(optarg,"emmc")==0) {
				//fprintf(stderr,"eMMC mode\n");
				mode=EMMC_M;
			} else if(strcasecmp(optarg,"nand")==0) {
				//fprintf(stderr,"NAND mode\n");
				mode=NAND_M;
			} else if(strcasecmp(optarg,"spi")==0) {
				//fprintf(stderr,"SPI mode\n");
				mode=SPI_M;
			} else {
				fprintf(stderr,"Unknown mode\n");
			}

			break;

		case 't':
			if(strcasecmp(optarg,"data")==0) {
				//fprintf(stderr,"DATA type\n");
				type=DATA;
			} else if(strcasecmp(optarg,"env")==0) {
				//fprintf(stderr,"Environment type\n");
				type=ENV;
			} else if(strcasecmp(optarg,"uboot")==0) {
				//fprintf(stderr,"uBoot type\n");
				type=UBOOT;
			} else if(strcasecmp(optarg,"pack")==0) {
				//fprintf(stderr,"Pack type\n");
				type=PACK;
			} else {
				fprintf(stderr,"Unknown type\n");
			}

			break;
		case 'e':
			erase_tag = 1;
			erase_read_len=strtoul(optarg,NULL,0);
			break;
		/* Optional args */
		case 'z':
			print_using();
			return 0;
			break;


		/* Error handle: Mainly missing arg or illegal option */
		case '?':
			fprintf(stderr, "Illegal option:-%c\n", isprint(optopt)?optopt:'#');
			break;
		default:
			fprintf(stderr, "Not supported option\n");
			fprintf(stderr, "Try 'nuwriter -h' for more information.\n");
			break;
		}
	}


	if(strlen(DDR_fileName)==0) {
		fprintf(stderr, "Not setting DDR file\n");
		return -1;
	}
	if(mode==-1) {
		fprintf(stderr, "Not setting mode\n");
		return -1;
	}

	if(mode==SDRAM_M) {
		if(exe_addr==0xffffffff) {
			fprintf(stderr, "Not setting execute/download address(-a [Address])\n");
			return -1;
		}
		if(strlen(write_file)==0) {
			fprintf(stderr, "Not setting DDR file(-m [DDR])\n");
			return -1;
		}

	}

	if((mode==NAND_M || mode==SPI_M || mode==EMMC_M) && type!=PACK) {
		if(write_tag==1 && read_tag==1) {
			fprintf(stderr, "Cannot write & read Nand flash at same time\n");
			return -1;
		}

		if(read_tag==1) {

			if(exe_addr==0xffffffff) {
				fprintf(stderr, "Not setting start address(-a [value])\n");
				return -1;
			}
			if(erase_tag!=1) {
				fprintf(stderr, "Not setting length(-e [value])\n");
				return -1;
			}
			erase_tag=0;
		}

		if(write_tag==1) {
			if(exe_addr==0xffffffff) {
				fprintf(stderr, "Not setting start address(-a [value])\n");
				return -1;
			}
			if(strlen(write_file)==0 && strlen(write_file)==0) {
				fprintf(stderr,"Not setting read/write file(-w [file])\n");
				return -1;
			}
			if(type==-1 && strlen(write_file)!=0) {
				fprintf(stderr, "Not setting type(-t [type])\n");
				return -1;
			}
		}
	}

	if(type==PACK) {
		if(verify_tag==1)
			fprintf(stderr, "Pack cannot supported verify (-v)\n");
	}

	libusb_init(NULL);
	while(1)
	{
		if ((usb_bus_cmdline !=-1) && (usb_port_cmdline != -1)) {
			char *c=strrchr(write_file, '/');
			c=(c==NULL)? write_file : c+1;
			printf("[Connect new device to Bus %d port %d] file: '%s'\n",
					usb_bus_cmdline, usb_port_cmdline, c);
			// wait for new device
			while(!NUC_IsDeviceConnectedToBP(usb_bus_cmdline, usb_port_cmdline))
				sleep(1);
		}

		usb_bus_auto = usb_bus_cmdline;
		usb_port_auto = usb_port_cmdline;

		if (execute_on_finish_len!=0)
		{
			execute_on_finish_buf[execute_on_finish_len] = 0;
			system(execute_on_finish_buf);
		}

		if (ParseFlashType()< 0) {
			printf("Failed\n");
			exitcode = -1;
		}
		else {
			printf("Done\n");
			exitcode = 0;
		}

		NUC_CloseUsb();
		if (execute_on_finish_len!=0)
		{
			snprintf(&execute_on_finish_buf[execute_on_finish_len],
					sizeof(execute_on_finish_buf) - execute_on_finish_len,
					" %d", exitcode);
			system(execute_on_finish_buf);
		}
		if ((usb_bus_cmdline ==-1) || (usb_port_cmdline == -1))
			break;

		printf("[Unplug device from Bus %d port %d]\n", usb_bus_cmdline, usb_port_cmdline);

		// wait for unplug
		while(NUC_IsDeviceConnectedToBP(usb_bus_cmdline, usb_port_cmdline))
			sleep(1);
		//sleep(4);
	}

	libusb_exit(NULL);
	return exitcode;
}
