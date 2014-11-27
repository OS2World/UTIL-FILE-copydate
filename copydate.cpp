#include <stdio.h>
#include <stdlib.h>
#include <io.h>

#define PROGNAME "copydate"
void usage(void) {
	printf("%s - copy the date&time of a file to another\n",PROGNAME);
	printf("usage: %s <orgfile> <dstfile>\n",PROGNAME);
	printf(" orgfile   The from the date is copied\n");
	printf(" dstfile   The file that receives the date\n");
	exit(1);
}


#ifdef __MSDOS__
#include <dos.h>
#include <fcntl.h>

void copydate(const char *srcfile, const char *dstfile) {
	int srchandle=open(srcfile,O_RDONLY);
	if(srchandle<0) {
		fprintf(stderr,"%s: could not open %s\n",PROGNAME,srcfile);
		exit(5);
	}
	struct ftime dt;
	if(getftime(srchandle,&dt)) {
		fprintf(stderr,"%s: could get date of %s\n",PROGNAME,srcfile);
		exit(5);
	}
	close(srchandle);

	int dsthandle=open(dstfile,O_RDONLY);
	if(srchandle<0) {
		fprintf(stderr,"%s: could not open %s\n",PROGNAME,dstfile);
		exit(5);
	}
	if(setftime(dsthandle,&dt)) {
		fprintf(stderr,"%s: could set date of %s\n",PROGNAME,dstfile);
		exit(5);
	}
	close(dsthandle);
}
#else
//OS/2
#define INCL_DOSFILEMGR
#include <os2.h>

void copydate(const char *srcfile, const char *dstfile) {
	HFILE hfileSrc;
	APIRET rc;
	ULONG dontCare;

	//open sourcefile
	rc = DosOpen((PSZ)srcfile,
		     &hfileSrc,
		     &dontCare,
		     0,  //filesize
		     0,  //attributes
		     OPEN_ACTION_FAIL_IF_NEW|OPEN_ACTION_OPEN_IF_EXISTS,
		     OPEN_FLAGS_NO_LOCALITY|OPEN_SHARE_DENYNONE|OPEN_ACCESS_READONLY,
                     (PEAOP2)NULL
		    );
	if(rc!=0) {
		fprintf(stderr,"%s: could not open %s (rc=%ld)\n",PROGNAME,srcfile,rc);
		exit(5);
	}

	//get date of sourcefile
	FILESTATUS3 fs3src;
	rc = DosQueryFileInfo(hfileSrc,
			      FIL_STANDARD,
			      (PVOID)&fs3src,
			      sizeof(fs3src)
			     );
	if(rc!=0) {
		fprintf(stderr,"%s: could not get date of %s (rc=%ld)\n",PROGNAME,srcfile,rc);
		exit(5);
	}

	//close sourcefile
	rc = DosClose(hfileSrc);
	if(rc!=0) {
		fprintf(stderr,"%s: could not close %s\n",PROGNAME,srcfile);
		exit(5);
	}

	//open dstfile
	HFILE hfileDst;
	rc = DosOpen((PSZ)dstfile,
		     &hfileDst,
		     &dontCare,
		     0, //filesize
		     0, //attribute
		     OPEN_ACTION_FAIL_IF_NEW|OPEN_ACTION_OPEN_IF_EXISTS,
		     OPEN_FLAGS_NO_LOCALITY|OPEN_SHARE_DENYREADWRITE|OPEN_ACCESS_READWRITE,
		     (PEAOP2)NULL
		    );
	if(rc!=0) {
		fprintf(stderr,"%s: could not open %s (rc=%ld)\n",PROGNAME,dstfile,rc);
		exit(5);
	}

	//get old info of dstfile
	FILESTATUS3 fs3dst;
	rc = DosQueryFileInfo(hfileDst,
			      FIL_STANDARD,
			      (PVOID)&fs3dst,
			      sizeof(fs3dst)
			     );
	if(rc!=0) {
		fprintf(stderr,"%s: could not get date of %s (rc=%ld)\n",PROGNAME,dstfile,rc);
		exit(5);
	}

	//make new fs
	fs3dst.fdateCreation  = fs3src.fdateCreation;
	fs3dst.ftimeCreation  = fs3src.ftimeCreation;
	fs3dst.fdateLastWrite = fs3src.fdateLastWrite;
	fs3dst.ftimeLastWrite = fs3src.ftimeLastWrite;

	//set the date of dstfile
	rc = DosSetFileInfo(hfileDst,
			    FIL_STANDARD,
			    (PVOID)&fs3dst,
			    sizeof(fs3dst)
			   );
	if(rc!=0) {
		fprintf(stderr,"%s: could not set date of %s (rc=%ld)\n",PROGNAME,dstfile,rc);
		exit(5);
	}

	//close dstfile
	rc = DosClose(hfileDst);
	if(rc!=0) {
		fprintf(stderr,"%s: could not close %s\n",PROGNAME,dstfile);
		exit(5);
	}

	//phew!
}
#endif

int main(int argc, char *argv[]) {
	if(argc!=3) usage();

	if(access(argv[1],0)) {
		fprintf(stderr,"%s: %s does not exist\n",PROGNAME,argv[1]);
		exit(2);
	}

	if(access(argv[2],02)) {
		fprintf(stderr,"%s: %s does not exist or not writable\n",PROGNAME,argv[2]);
		exit(3);
	}

	copydate(argv[1],argv[2]);

	return 0;
}
