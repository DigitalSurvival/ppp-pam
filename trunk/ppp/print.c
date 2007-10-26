/* Copyright (c) 2007, Thomas Fors
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "print.h"
#include "cmdline.h"

static char *buf = NULL;
 
void printInit() {
	
}   

void printCleanup() {
	free(buf);
}

char *mpToDecimalString(mp_int *mp, char groupChar) {
	int len = mp_radix_size(mp, 10);
	int nCommas = 0;

	free(buf);
	buf = malloc(len + len/3 + 2);
	mp_toradix(mp, (unsigned char *)buf, 10);

	len = strlen(buf);
           
	if (groupChar) {
		int nGroups = len / 3;
		int firstGroupLen = len % 3;
		nCommas = nGroups;
		if (firstGroupLen == 0) {
			nCommas--;
		}
	}
	
	               
	if (nCommas > 0) {
		int i, pos;
		int offset = nCommas;
		buf[len+offset] = '\x00';
		for (i=0; i<len; i++) {
			pos = len - 1 - i;
			if (i>0 && i%3==0) {
				buf[pos+offset] = ',';
				offset--;
			}
			buf[pos+offset] = buf[pos];
		}
	}

	return buf;	
}

void printCard(mp_int *nCard) {
	char groupChar = ',';
	mp_int start;
	mp_init(&start);
	calculatePasscodeNumberFromCardColRow(nCard, 0, 0, &start);

	char buf[70*4];
	getPasscodeBlock(&start, 70, buf);
	mp_clear(&start);
	
	char hname[39];
	strncpy(hname, hostname(), 38);
	
	mp_int n;
	mp_init(&n);
	mp_add_d(nCard, 1, &n);
	char *cardnumber = mpToDecimalString(&n, groupChar);
	char *cn = cardnumber;
	mp_clear(&n);
	
	if (strlen(hname) + strlen(cardnumber) + 3 > 38) {
		if (strlen(hname) > 27) {
			hname[27] = '\x00';
		}
		int ellipses = strlen(cardnumber) - (38 - strlen(hname) - 3);
		if (ellipses > 0) {
			cn = cardnumber+ellipses;
			cn[0] = cn[1] = cn[2] = '.';
			/* When truncating the card number, make sure we don't
			 * begin with a comma after the ellipses
			 */
			if (cn[3] == groupChar) {
				cn[3] = '.';
				cn++;
			}
		}
	}
	
	printf("%s", hname);
	int j;
	for (j=0; j<38-strlen(hname)-strlen(cn)-2; j++)
		printf(" ");
	printf("[%s]\n", cn);
	printf("      A    B    C    D    E    F    G\n");
	         
	j = 0;
	int r, c;
	for (r=1; r<=10; r++) {
		printf("%2d: ", r);
		for (c=0; c<7; c++) {
			if (c) printf(" ");
			printf("%c%c%c%c", buf[j*4+0], buf[j*4+1], buf[j*4+2], buf[j*4+3]);  
			j++;
		}
		printf("\n");
	}
	printf("\n");

	/* zero passcodes from memory */
	memset(buf, 0, 70*4);
}