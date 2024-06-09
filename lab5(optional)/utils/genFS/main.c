#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include "data.h"
#include "func.h"

int main(int argc, char *argv[]) {
	char driver[NAME_LENGTH];
	char srcFilePath[NAME_LENGTH];
	char destFilePath[NAME_LENGTH];

	stringCpy("fs.bin", driver, NAME_LENGTH - 1);
	format(driver, SECTOR_NUM, SECTORS_PER_BLOCK);

	stringCpy("/boot", destFilePath, NAME_LENGTH - 1);
	mkdir(driver, destFilePath);
	stringCpy(argv[1], srcFilePath, NAME_LENGTH - 1);
	stringCpy("/boot/initrd", destFilePath, NAME_LENGTH - 1);
	cp(driver, srcFilePath, destFilePath);
	
	stringCpy("/dev", destFilePath, NAME_LENGTH - 1);
	mkdir(driver, destFilePath);
	stringCpy("/dev/stdin", destFilePath, NAME_LENGTH - 1);
	touch(driver, destFilePath);
	stringCpy("/dev/stdout", destFilePath, NAME_LENGTH - 1);
	touch(driver, destFilePath);

	stringCpy("/usr", destFilePath, NAME_LENGTH - 1);
	mkdir(driver, destFilePath);

	stringCpy("/data/", destFilePath, NAME_LENGTH - 1);
	mkdir(driver, destFilePath);
	stringCpy("/data/test.txt", destFilePath, NAME_LENGTH - 1);
	touch(driver, destFilePath);

	stringCpy("/data/dir1", destFilePath, NAME_LENGTH - 1);
	mkdir(driver, destFilePath);
	stringCpy("/data/dir1/dir11", destFilePath, NAME_LENGTH - 1);
	mkdir(driver, destFilePath);
	stringCpy("/data/dir1/dir11/test.txt", destFilePath, NAME_LENGTH - 1);
	touch(driver, destFilePath);

	stringCpy("/data/dir2", destFilePath, NAME_LENGTH - 1);
	mkdir(driver, destFilePath);
	stringCpy("/data/dir2/test.txt", destFilePath, NAME_LENGTH - 1);
	touch(driver, destFilePath);

	return 0;
}
