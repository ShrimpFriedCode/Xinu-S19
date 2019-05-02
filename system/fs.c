#include <xinu.h>
#include <kernel.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef FS
#include <fs.h>

static struct fsystem fsd;
int dev0_numblocks;
int dev0_blocksize;
char *dev0_blocks;

extern int dev0;

char block_cache[512];

#define SB_BLK 0
#define BM_BLK 1
#define RT_BLK 2

#define NUM_FD 16
struct filetable oft[NUM_FD];
int next_open_fd = 0;


#define INODES_PER_BLOCK (fsd.blocksz / sizeof(struct inode))
#define NUM_INODE_BLOCKS (( (fsd.ninodes % INODES_PER_BLOCK) == 0) ? fsd.ninodes / INODES_PER_BLOCK : (fsd.ninodes / INODES_PER_BLOCK) + 1)
#define FIRST_INODE_BLOCK 2

int fs_fileblock_to_diskblock(int dev, int fd, int fileblock);

/* YOUR CODE GOES HERE */

int fs_fileblock_to_diskblock(int dev, int fd, int fileblock) {
	int diskblock;

	if (fileblock >= INODEBLOCKS - 2) {
		printf("No indirect block support\n");
		return SYSERR;
	}

	diskblock = oft[fd].in.blocks[fileblock]; //get the logical block address

	return diskblock;
}

/* read in an inode and fill in the pointer */
int
fs_get_inode_by_num(int dev, int inode_number, struct inode *in) {
	int bl, inn;
	int inode_off;

	if (dev != 0) {
		printf("Unsupported device\n");
		return SYSERR;
	}
	if (inode_number > fsd.ninodes) {
		printf("fs_get_inode_by_num: inode %d out of range\n", inode_number);
		return SYSERR;
	}

	bl = inode_number / INODES_PER_BLOCK;
	inn = inode_number % INODES_PER_BLOCK;
	bl += FIRST_INODE_BLOCK;

	inode_off = inn * sizeof(struct inode);

	/*
printf("in_no: %d = %d/%d\n", inode_number, bl, inn);
printf("inn*sizeof(struct inode): %d\n", inode_off);
*/

	bs_bread(dev0, bl, 0, &block_cache[0], fsd.blocksz);
	memcpy(in, &block_cache[inode_off], sizeof(struct inode));

	return OK;

}

int
fs_put_inode_by_num(int dev, int inode_number, struct inode *in) {
	int bl, inn;

	if (dev != 0) {
		printf("Unsupported device\n");
		return SYSERR;
	}
	if (inode_number > fsd.ninodes) {
		printf("fs_put_inode_by_num: inode %d out of range\n", inode_number);
		return SYSERR;
	}

	bl = inode_number / INODES_PER_BLOCK;
	inn = inode_number % INODES_PER_BLOCK;
	bl += FIRST_INODE_BLOCK;

	/*
printf("in_no: %d = %d/%d\n", inode_number, bl, inn);
*/

	bs_bread(dev0, bl, 0, block_cache, fsd.blocksz);
	memcpy(&block_cache[(inn*sizeof(struct inode))], in, sizeof(struct inode));
	bs_bwrite(dev0, bl, 0, block_cache, fsd.blocksz);

	return OK;
}

int fs_mkfs(int dev, int num_inodes) {
	int i;

	if (dev == 0) {
		fsd.nblocks = dev0_numblocks;
		fsd.blocksz = dev0_blocksize;
	}
	else {
		printf("Unsupported device\n");
		return SYSERR;
	}

	if (num_inodes < 1) {
		fsd.ninodes = DEFAULT_NUM_INODES;
	}
	else {
		fsd.ninodes = num_inodes;
	}

	i = fsd.nblocks;
	while ( (i % 8) != 0) {i++;}
	fsd.freemaskbytes = i / 8; 

	if ((fsd.freemask = getmem(fsd.freemaskbytes)) == (void *)SYSERR) {
		printf("fs_mkfs memget failed.\n");
		return SYSERR;
	}

	/* zero the free mask */
	for(i=0;i<fsd.freemaskbytes;i++) {
		fsd.freemask[i] = '\0';
	}

	fsd.inodes_used = 0;

	/* write the fsystem block to SB_BLK, mark block used */
	fs_setmaskbit(SB_BLK);
	bs_bwrite(dev0, SB_BLK, 0, &fsd, sizeof(struct fsystem));

	/* write the free block bitmask in BM_BLK, mark block used */
	fs_setmaskbit(BM_BLK);
	bs_bwrite(dev0, BM_BLK, 0, fsd.freemask, fsd.freemaskbytes);

	return 1;
}

void
fs_print_fsd(void) {

	printf("fsd.ninodes: %d\n", fsd.ninodes);
	printf("sizeof(struct inode): %d\n", sizeof(struct inode));
	printf("INODES_PER_BLOCK: %d\n", INODES_PER_BLOCK);
	printf("NUM_INODE_BLOCKS: %d\n", NUM_INODE_BLOCKS);
}

/* specify the block number to be set in the mask */
int fs_setmaskbit(int b) {
	int mbyte, mbit;
	mbyte = b / 8;
	mbit = b % 8;

	fsd.freemask[mbyte] |= (0x80 >> mbit);
	return OK;
}

/* specify the block number to be read in the mask */
int fs_getmaskbit(int b) {
	int mbyte, mbit;
	mbyte = b / 8;
	mbit = b % 8;

	return( ( (fsd.freemask[mbyte] << mbit) & 0x80 ) >> 7);
	return OK;

}

/* specify the block number to be unset in the mask */
int fs_clearmaskbit(int b) {
	int mbyte, mbit, invb;
	mbyte = b / 8;
	mbit = b % 8;

	invb = ~(0x80 >> mbit);
	invb &= 0xFF;

	fsd.freemask[mbyte] &= invb;
	return OK;
}

/* This is maybe a little overcomplicated since the lowest-numbered
block is indicated in the high-order bit.  Shift the byte by j
positions to make the match in bit7 (the 8th bit) and then shift
that value 7 times to the low-order bit to print.  Yes, it could be
the other way...  */
void fs_printfreemask(void) {
	int i,j;

	for (i=0; i < fsd.freemaskbytes; i++) {
		for (j=0; j < 8; j++) {
			printf("%d", ((fsd.freemask[i] << j) & 0x80) >> 7);
		}
		if ( (i % 8) == 7) {
			printf("\n");
		}
	}
	printf("\n");
}

int fs_open(char *filename, int flags) {
	int itter;

	for (itter = 0; itter<fsd.root_dir.numentries; itter++) {//find directory entry for filename
		if (strcmp(fsd.root_dir.entry[itter].name, filename) == 0) {//if directory entry has our filename, open

			int fd = next_open_fd;

			while (++next_open_fd<NUM_FD && oft[next_open_fd].state == FSTATE_OPEN); //ensure open fd for file opening
			//open file
			oft[fd].state = FSTATE_OPEN;
			oft[fd].fileptr = 0;
			oft[fd].mode = flags;
			bs_bread(dev0, fsd.root_dir.entry[itter].inode_num, 0, &oft[fd].in, sizeof(struct inode));
			kprintf("%d\n", oft[fd].in.nlink);

			return fd;
		}
	}
	//else, file needs to be created
	return fs_create(filename, O_CREAT);
}

int fs_close(int fd) {//change state ti closed
	oft[fd].state = FSTATE_CLOSED;
	return OK;
}

int fs_create(char *filename, int mode) {
	int itter;
	int fd = next_open_fd;
	int max = RT_BLK + DEFAULT_NUM_INODES;
	int inode_number = max;

	for(itter = RT_BLK; itter < max; itter++){//find available blocks
		if (fs_getmaskbit(itter) == 0){
			//if available block found
			inode_number = itter;//record availibility
		}
	}

	if (inode_number == max) {//no blocks available
		return SYSERR;//err
	}
	
	strcpy(fsd.root_dir.entry[fsd.root_dir.numentries].name, filename); //add filename to dirs
	fsd.root_dir.entry[fsd.root_dir.numentries].inode_num = inode_number;//record inode number
	fsd.root_dir.numentries++;//increase numentries

	while (++next_open_fd < NUM_FD && oft[next_open_fd].state == FSTATE_OPEN);//obtain an open fd
	
	if (next_open_fd == NUM_FD) {//err if no fd can be obtained
		return SYSERR;
	}

	//set attributes of fd
	oft[fd].state = FSTATE_OPEN;
	oft[fd].fileptr = 0;
	oft[fd].mode = mode;
	//set attributes of inode
	oft[fd].in.id = inode_number;
	oft[fd].in.type = INODE_TYPE_FILE;
	oft[fd].in.nlink = 0;
	oft[fd].in.size = 0;

	//write
	bs_bwrite(dev0, SB_BLK, 0, &fsd, sizeof(struct fsystem));
	bs_bwrite(dev0, oft[fd].in.id, 0, &oft[fd].in, sizeof(struct inode));
	bs_bwrite(dev0, BM_BLK, 0, fsd.freemask, fsd.freemaskbytes);
	//return
	return fd;
}

int fs_seek(int fd, int offset) {//set offset to fptr
	oft[fd].fileptr += offset;
	return OK;
}

int fs_read(int fd, void *buf, int nbytes) {
	int blockIndex;
	int low, high;
	int byt;

	int ret = 0;
	while (nbytes > 0) {
		blockIndex = oft[fd].fileptr / fsd.blocksz;//get index of block

		if(blockIndex >= oft[fd].in.nlink) {//out of bounds
			return SYSERR;
		}

		low = oft[fd].fileptr % fsd.blocksz; //small
		//range
		high = low + nbytes; //large

		if (high > fsd.blocksz){//if larger than current block size
			high = fsd.blocksz;//change size
		}

		byt = high - low; //bytes we want to read


		bs_bread(dev0, oft[fd].in.blocks[blockIndex], low, buf + ret, byt);

		//move file pointer
		oft[fd].fileptr += byt;
		//read bytes
		nbytes -= byt;
		ret += byt;
	}
	return ret;
}

int fs_write(int fd, void *buf, int nbytes) {
	int blockIndex;
	int low, high;
	int byt;
	int dataBlock;
	int ret = 0;


	while (nbytes > 0) {
		blockIndex = oft[fd].fileptr / fsd.blocksz;//get index of block

		while(blockIndex >= oft[fd].in.nlink){

			if (oft[fd].in.nlink == INODEBLOCKS) {//not enough room
				return SYSERR;
			}

			low = oft[fd].fileptr % fsd.blocksz;//small
			//range
			high = low + nbytes; //large

			if (high > fsd.blocksz){
				high = fsd.blocksz;//if high is larger than block size, set accordingly
			}

			byt = high - low;//to write

			int openDataBlock = MDEV_NUM_BLOCKS;

			for (dataBlock = RT_BLK + DEFAULT_NUM_INODES; dataBlock < MDEV_NUM_BLOCKS; dataBlock++) {
				if (fs_getmaskbit(dataBlock) == 0){
					openDataBlock = dataBlock;
				}
			}

			if (openDataBlock == MDEV_NUM_BLOCKS) {//no open blocks found
				return SYSERR;
			}

			fs_setmaskbit(openDataBlock);
			oft[fd].in.blocks[oft[fd].in.nlink] = openDataBlock;
			oft[fd].in.nlink++;
		}
		//write bytes, adjust counters
		bs_bwrite(dev0, oft[fd].in.blocks[blockIndex], low, buf + ret, byt);

		oft[fd].fileptr += byt;
		nbytes -= byt;
		
		ret += byt;
	}
	//write
	bs_bwrite(dev0, oft[fd].in.id, 0, &oft[fd].in, sizeof(struct inode));
	bs_bwrite(dev0, BM_BLK, 0, fsd.freemask, fsd.freemaskbytes);
	return ret;
}

#endif /* FS */