/*
 * nixio - Linux I/O library for lua
 *
 *   Copyright (C) 2009 Steven Barth <steven@midlink.org>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "nixio.h"
#include <stdlib.h>

const char nixio__bin2hex[16] = {
'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

static unsigned char nixio__b64encode_tbl[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static unsigned char nixio__b64decode_tbl[] = {
	0x3e, 0xff, 0xff, 0xff, 0x3f, 0x34, 0x35, 0x36,
	0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0xff,
	0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0x01,
	0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
	0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11,
	0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1a, 0x1b,
	0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
	0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
	0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33
};

static const uint32_t nixio__crc32_tbl[] = {
	0x00000000U, 0x77073096U, 0xee0e612cU, 0x990951baU, 0x076dc419U,
	0x706af48fU, 0xe963a535U, 0x9e6495a3U, 0x0edb8832U, 0x79dcb8a4U,
	0xe0d5e91eU, 0x97d2d988U, 0x09b64c2bU, 0x7eb17cbdU, 0xe7b82d07U,
	0x90bf1d91U, 0x1db71064U, 0x6ab020f2U, 0xf3b97148U, 0x84be41deU,
	0x1adad47dU, 0x6ddde4ebU, 0xf4d4b551U, 0x83d385c7U, 0x136c9856U,
	0x646ba8c0U, 0xfd62f97aU, 0x8a65c9ecU, 0x14015c4fU, 0x63066cd9U,
	0xfa0f3d63U, 0x8d080df5U, 0x3b6e20c8U, 0x4c69105eU, 0xd56041e4U,
	0xa2677172U, 0x3c03e4d1U, 0x4b04d447U, 0xd20d85fdU, 0xa50ab56bU,
	0x35b5a8faU, 0x42b2986cU, 0xdbbbc9d6U, 0xacbcf940U, 0x32d86ce3U,
	0x45df5c75U, 0xdcd60dcfU, 0xabd13d59U, 0x26d930acU, 0x51de003aU,
	0xc8d75180U, 0xbfd06116U, 0x21b4f4b5U, 0x56b3c423U, 0xcfba9599U,
	0xb8bda50fU, 0x2802b89eU, 0x5f058808U, 0xc60cd9b2U, 0xb10be924U,
	0x2f6f7c87U, 0x58684c11U, 0xc1611dabU, 0xb6662d3dU, 0x76dc4190U,
	0x01db7106U, 0x98d220bcU, 0xefd5102aU, 0x71b18589U, 0x06b6b51fU,
	0x9fbfe4a5U, 0xe8b8d433U, 0x7807c9a2U, 0x0f00f934U, 0x9609a88eU,
	0xe10e9818U, 0x7f6a0dbbU, 0x086d3d2dU, 0x91646c97U, 0xe6635c01U,
	0x6b6b51f4U, 0x1c6c6162U, 0x856530d8U, 0xf262004eU, 0x6c0695edU,
	0x1b01a57bU, 0x8208f4c1U, 0xf50fc457U, 0x65b0d9c6U, 0x12b7e950U,
	0x8bbeb8eaU, 0xfcb9887cU, 0x62dd1ddfU, 0x15da2d49U, 0x8cd37cf3U,
	0xfbd44c65U, 0x4db26158U, 0x3ab551ceU, 0xa3bc0074U, 0xd4bb30e2U,
	0x4adfa541U, 0x3dd895d7U, 0xa4d1c46dU, 0xd3d6f4fbU, 0x4369e96aU,
	0x346ed9fcU, 0xad678846U, 0xda60b8d0U, 0x44042d73U, 0x33031de5U,
	0xaa0a4c5fU, 0xdd0d7cc9U, 0x5005713cU, 0x270241aaU, 0xbe0b1010U,
	0xc90c2086U, 0x5768b525U, 0x206f85b3U, 0xb966d409U, 0xce61e49fU,
	0x5edef90eU, 0x29d9c998U, 0xb0d09822U, 0xc7d7a8b4U, 0x59b33d17U,
	0x2eb40d81U, 0xb7bd5c3bU, 0xc0ba6cadU, 0xedb88320U, 0x9abfb3b6U,
	0x03b6e20cU, 0x74b1d29aU, 0xead54739U, 0x9dd277afU, 0x04db2615U,
	0x73dc1683U, 0xe3630b12U, 0x94643b84U, 0x0d6d6a3eU, 0x7a6a5aa8U,
	0xe40ecf0bU, 0x9309ff9dU, 0x0a00ae27U, 0x7d079eb1U, 0xf00f9344U,
	0x8708a3d2U, 0x1e01f268U, 0x6906c2feU, 0xf762575dU, 0x806567cbU,
	0x196c3671U, 0x6e6b06e7U, 0xfed41b76U, 0x89d32be0U, 0x10da7a5aU,
	0x67dd4accU, 0xf9b9df6fU, 0x8ebeeff9U, 0x17b7be43U, 0x60b08ed5U,
	0xd6d6a3e8U, 0xa1d1937eU, 0x38d8c2c4U, 0x4fdff252U, 0xd1bb67f1U,
	0xa6bc5767U, 0x3fb506ddU, 0x48b2364bU, 0xd80d2bdaU, 0xaf0a1b4cU,
	0x36034af6U, 0x41047a60U, 0xdf60efc3U, 0xa867df55U, 0x316e8eefU,
	0x4669be79U, 0xcb61b38cU, 0xbc66831aU, 0x256fd2a0U, 0x5268e236U,
	0xcc0c7795U, 0xbb0b4703U, 0x220216b9U, 0x5505262fU, 0xc5ba3bbeU,
	0xb2bd0b28U, 0x2bb45a92U, 0x5cb36a04U, 0xc2d7ffa7U, 0xb5d0cf31U,
	0x2cd99e8bU, 0x5bdeae1dU, 0x9b64c2b0U, 0xec63f226U, 0x756aa39cU,
	0x026d930aU, 0x9c0906a9U, 0xeb0e363fU, 0x72076785U, 0x05005713U,
	0x95bf4a82U, 0xe2b87a14U, 0x7bb12baeU, 0x0cb61b38U, 0x92d28e9bU,
	0xe5d5be0dU, 0x7cdcefb7U, 0x0bdbdf21U, 0x86d3d2d4U, 0xf1d4e242U,
	0x68ddb3f8U, 0x1fda836eU, 0x81be16cdU, 0xf6b9265bU, 0x6fb077e1U,
	0x18b74777U, 0x88085ae6U, 0xff0f6a70U, 0x66063bcaU, 0x11010b5cU,
	0x8f659effU, 0xf862ae69U, 0x616bffd3U, 0x166ccf45U, 0xa00ae278U,
	0xd70dd2eeU, 0x4e048354U, 0x3903b3c2U, 0xa7672661U, 0xd06016f7U,
	0x4969474dU, 0x3e6e77dbU, 0xaed16a4aU, 0xd9d65adcU, 0x40df0b66U,
	0x37d83bf0U, 0xa9bcae53U, 0xdebb9ec5U, 0x47b2cf7fU, 0x30b5ffe9U,
	0xbdbdf21cU, 0xcabac28aU, 0x53b39330U, 0x24b4a3a6U, 0xbad03605U,
	0xcdd70693U, 0x54de5729U, 0x23d967bfU, 0xb3667a2eU, 0xc4614ab8U,
	0x5d681b02U, 0x2a6f2b94U, 0xb40bbe37U, 0xc30c8ea1U, 0x5a05df1bU,
	0x2d02ef8dU
};

static const uint8_t nixio__crc16_hi[] = {
	0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x01, 0xc0,
	0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41,
	0x00, 0xc1, 0x81, 0x40, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0,
	0x80, 0x41, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81, 0x40,
	0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1,
	0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x01, 0xc0, 0x80, 0x41,
	0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1,
	0x81, 0x40, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41,
	0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x01, 0xc0,
	0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x00, 0xc1, 0x81, 0x40,
	0x01, 0xc0, 0x80, 0x41, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1,
	0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81, 0x40,
	0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x01, 0xc0,
	0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x00, 0xc1, 0x81, 0x40,
	0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0,
	0x80, 0x41, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81, 0x40,
	0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x01, 0xc0,
	0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41,
	0x00, 0xc1, 0x81, 0x40, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0,
	0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41,
	0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0,
	0x80, 0x41, 0x00, 0xc1, 0x81, 0x40, 0x00, 0xc1, 0x81, 0x40,
	0x01, 0xc0, 0x80, 0x41, 0x01, 0xc0, 0x80, 0x41, 0x00, 0xc1,
	0x81, 0x40, 0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41,
	0x00, 0xc1, 0x81, 0x40, 0x01, 0xc0, 0x80, 0x41, 0x01, 0xc0,
	0x80, 0x41, 0x00, 0xc1, 0x81, 0x40
};

static const uint8_t nixio__crc16_lo[] = {
	0x00, 0xc0, 0xc1, 0x01, 0xc3, 0x03, 0x02, 0xc2, 0xc6, 0x06,
	0x07, 0xc7, 0x05, 0xc5, 0xc4, 0x04, 0xcc, 0x0c, 0x0d, 0xcd,
	0x0f, 0xcf, 0xce, 0x0e, 0x0a, 0xca, 0xcb, 0x0b, 0xc9, 0x09,
	0x08, 0xc8, 0xd8, 0x18, 0x19, 0xd9, 0x1b, 0xdb, 0xda, 0x1a,
	0x1e, 0xde, 0xdf, 0x1f, 0xdd, 0x1d, 0x1c, 0xdc, 0x14, 0xd4,
	0xd5, 0x15, 0xd7, 0x17, 0x16, 0xd6, 0xd2, 0x12, 0x13, 0xd3,
	0x11, 0xd1, 0xd0, 0x10, 0xf0, 0x30, 0x31, 0xf1, 0x33, 0xf3,
	0xf2, 0x32, 0x36, 0xf6, 0xf7, 0x37, 0xf5, 0x35, 0x34, 0xf4,
	0x3c, 0xfc, 0xfd, 0x3d, 0xff, 0x3f, 0x3e, 0xfe, 0xfa, 0x3a,
	0x3b, 0xfb, 0x39, 0xf9, 0xf8, 0x38, 0x28, 0xe8, 0xe9, 0x29,
	0xeb, 0x2b, 0x2a, 0xea, 0xee, 0x2e, 0x2f, 0xef, 0x2d, 0xed,
	0xec, 0x2c, 0xe4, 0x24, 0x25, 0xe5, 0x27, 0xe7, 0xe6, 0x26,
	0x22, 0xe2, 0xe3, 0x23, 0xe1, 0x21, 0x20, 0xe0, 0xa0, 0x60,
	0x61, 0xa1, 0x63, 0xa3, 0xa2, 0x62, 0x66, 0xa6, 0xa7, 0x67,
	0xa5, 0x65, 0x64, 0xa4, 0x6c, 0xac, 0xad, 0x6d, 0xaf, 0x6f,
	0x6e, 0xae, 0xaa, 0x6a, 0x6b, 0xab, 0x69, 0xa9, 0xa8, 0x68,
	0x78, 0xb8, 0xb9, 0x79, 0xbb, 0x7b, 0x7a, 0xba, 0xbe, 0x7e,
	0x7f, 0xbf, 0x7d, 0xbd, 0xbc, 0x7c, 0xb4, 0x74, 0x75, 0xb5,
	0x77, 0xb7, 0xb6, 0x76, 0x72, 0xb2, 0xb3, 0x73, 0xb1, 0x71,
	0x70, 0xb0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
	0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9c, 0x5c,
	0x5d, 0x9d, 0x5f, 0x9f, 0x9e, 0x5e, 0x5a, 0x9a, 0x9b, 0x5b,
	0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4b, 0x8b,
	0x8a, 0x4a, 0x4e, 0x8e, 0x8f, 0x4f, 0x8d, 0x4d, 0x4c, 0x8c,
	0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
	0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};


static const uint8_t nixio__dow_crc_tbl[] = {
	0x00, 0x5e, 0xbc, 0xe2, 0x61, 0x3f, 0xdd, 0x83, 
	0xc2, 0x9c, 0x7e, 0x20, 0xa3, 0xfd, 0x1f, 0x41, 
	0x9d, 0xc3, 0x21, 0x7f, 0xfc, 0xa2, 0x40, 0x1e, 
	0x5f, 0x01, 0xe3, 0xbd, 0x3e, 0x60, 0x82, 0xdc, 
	0x23, 0x7d, 0x9f, 0xc1, 0x42, 0x1c, 0xfe, 0xa0, 
	0xe1, 0xbf, 0x5d, 0x03, 0x80, 0xde, 0x3c, 0x62, 
	0xbe, 0xe0, 0x02, 0x5c, 0xdf, 0x81, 0x63, 0x3d, 
	0x7c, 0x22, 0xc0, 0x9e, 0x1d, 0x43, 0xa1, 0xff, 
	0x46, 0x18, 0xfa, 0xa4, 0x27, 0x79, 0x9b, 0xc5, 
	0x84, 0xda, 0x38, 0x66, 0xe5, 0xbb, 0x59, 0x07, 
	0xdb, 0x85, 0x67, 0x39, 0xba, 0xe4, 0x06, 0x58, 
	0x19, 0x47, 0xa5, 0xfb, 0x78, 0x26, 0xc4, 0x9a, 
	0x65, 0x3b, 0xd9, 0x87, 0x04, 0x5a, 0xb8, 0xe6, 
	0xa7, 0xf9, 0x1b, 0x45, 0xc6, 0x98, 0x7a, 0x24, 
	0xf8, 0xa6, 0x44, 0x1a, 0x99, 0xc7, 0x25, 0x7b, 
	0x3a, 0x64, 0x86, 0xd8, 0x5b, 0x05, 0xe7, 0xb9, 
	0x8c, 0xd2, 0x30, 0x6e, 0xed, 0xb3, 0x51, 0x0f, 
	0x4e, 0x10, 0xf2, 0xac, 0x2f, 0x71, 0x93, 0xcd, 
	0x11, 0x4f, 0xad, 0xf3, 0x70, 0x2e, 0xcc, 0x92, 
	0xd3, 0x8d, 0x6f, 0x31, 0xb2, 0xec, 0x0e, 0x50, 
	0xaf, 0xf1, 0x13, 0x4d, 0xce, 0x90, 0x72, 0x2c, 
	0x6d, 0x33, 0xd1, 0x8f, 0x0c, 0x52, 0xb0, 0xee, 
	0x32, 0x6c, 0x8e, 0xd0, 0x53, 0x0d, 0xef, 0xb1, 
	0xf0, 0xae, 0x4c, 0x12, 0x91, 0xcf, 0x2d, 0x73, 
	0xca, 0x94, 0x76, 0x28, 0xab, 0xf5, 0x17, 0x49, 
	0x08, 0x56, 0xb4, 0xea, 0x69, 0x37, 0xd5, 0x8b, 
	0x57, 0x09, 0xeb, 0xb5, 0x36, 0x68, 0x8a, 0xd4, 
	0x95, 0xcb, 0x29, 0x77, 0xf4, 0xaa, 0x48, 0x16, 
	0xe9, 0xb7, 0x55, 0x0b, 0x88, 0xd6, 0x34, 0x6a, 
	0x2b, 0x75, 0x97, 0xc9, 0x4a, 0x14, 0xf6, 0xa8, 
	0x74, 0x2a, 0xc8, 0x96, 0x15, 0x4b, 0xa9, 0xf7, 
	0xb6, 0xe8, 0x0a, 0x54, 0xd7, 0x89, 0x6b, 0x35, 
};

static int nixio_bin_crc32(lua_State *L) {
	size_t len;
	const char *buffer = luaL_checklstring(L, 1, &len);
	uint32_t value = luaL_optinteger(L, 2, 0);

	value = ~value;
	for (size_t i=0; i<len; i++) {
		value = nixio__crc32_tbl[(value ^ buffer[i]) & 0xffU ] ^ (value >> 8);
	}

	lua_pushinteger(L, (int)(value ^ 0xffffffffU));
	return 1;
}

/* implementation of CRC16 0x8005 polynomial, aka Modbus CRC
   source: https://github.com/stephane/libmodbus
*/
static int nixio_bin_crc16(lua_State *L) {
	size_t len;
	char *buffer = luaL_checklstring(L, 1, &len);
	uint16_t init = luaL_optinteger(L, 2, 0xffff);

	uint8_t crc_hi = init >> 8; /* high CRC byte initialized */
	uint8_t crc_lo = init & 0xff; /* low CRC byte initialized */
	uint8_t i; /* will index into CRC lookup */

	/* pass through message buffer */
	while (len--) {
		i = crc_hi ^ *buffer++; /* calculate the CRC  */
		crc_hi = crc_lo ^ nixio__crc16_hi[i];
		crc_lo = nixio__crc16_lo[i];
	}

	/* swap bytes to get the Modbus CRC */
	lua_pushinteger(L, crc_lo << 8 | crc_hi);
	return 1;
}

static int nixio_bin_dow_crc(lua_State *L) {
	size_t len;
	const char *buffer = luaL_checklstring(L, 1, &len);
	uint8_t value = (uint8_t)luaL_optinteger(L, 2, 0);

	for (size_t i=0; i<len; i++) {
		value = nixio__dow_crc_tbl[(value ^ buffer[i]) & 0xff];
	}

	lua_pushinteger(L, (int)value);
	return 1;
}

static int nixio_bin_hexlify(lua_State *L) {
	size_t len, lenout;
	luaL_checktype(L, 1, LUA_TSTRING);
	const unsigned char *data = (unsigned char*)lua_tolstring(L, 1, &len);

	if (len == 0) {
		lua_pushvalue(L, 1);
		return 1;
	}

	lenout = len * 2;
	luaL_argcheck(L, lenout > len, 1, "size overflow");

	char *out = malloc(lenout);
	if (!out) {
		return luaL_error(L, NIXIO_OOM);
	}

	for (size_t i=0; i < len; i++) {
		out[2*i]   = nixio__bin2hex[(data[i] & 0xf0) >> 4];
		out[2*i+1] = nixio__bin2hex[(data[i] & 0x0f)];
	}

	lua_pushlstring(L, out, lenout);
	free(out);

	return 1;
}


static int nixio_bin_unhexlify(lua_State *L) {
	size_t len, lenout;
	const char *data = luaL_checklstring(L, 1, &len);

	if (len == 0) {
		lua_pushvalue(L, 1);
		return 1;
	} else if (len % 2) {
		errno = EINVAL;
		return nixio__perror(L);
	}

	lenout = len / 2;
	char *out = malloc(lenout);
	if (!out) {
		return luaL_error(L, NIXIO_OOM);
	}

	for (size_t i=0; i < lenout; i++) {
		char c = data[2*i];
		if (c >= '0' && c <= '9') {
			out[i] = (c - '0') << 4;
		} else if (c >= 'a' && c <= 'f') {
			out[i] = (c - 'a' + 10) << 4;
		} else if (data[2*i] >= 'A' && c <= 'F') {
			out[i] = (c - 'A' + 10) << 4;
		} else {
			free(out);
			errno = EINVAL;
			return nixio__perror(L);
		}

		c = data[2*i+1];
		if (c >= '0' && c <= '9') {
			out[i] += c - '0';
		} else if (c >= 'a' && c <= 'f') {
			out[i] += c - 'a' + 10;
		} else if (c >= 'A' && c <= 'F') {
			out[i] += c - 'A' + 10;
		} else {
			free(out);
			errno = EINVAL;
			return nixio__perror(L);
		}
	}

	lua_pushlstring(L, out, lenout);
	free(out);

	return 1;
}

static int nixio_bin_b64encode(lua_State *L) {
	size_t len, lenout, pad, i;
	const uint8_t *data = (const uint8_t*)luaL_checklstring(L, 1, &len);

	lenout = len / 3;
	lenout *= 4;

	pad = len % 3;

	if (len == 0) {
		lua_pushvalue(L, 1);
		return 1;
	} else if (pad) {
		lenout += 4;
	}

	luaL_argcheck(L, lenout > len, 1, "size overflow");

	char *out = malloc(lenout);
	if (!out) {
		return luaL_error(L, NIXIO_OOM);
	}

	uint8_t *o = (uint8_t*)out;
	for (i = 0; i < len; i += 3) {
		uint32_t cv = (data[i] << 16) | (data[i+1] << 8) | data[i+2];
		*(o+3) = nixio__b64encode_tbl[ cv        & 0x3f];
		*(o+2) = nixio__b64encode_tbl[(cv >> 6)  & 0x3f];
		*(o+1) = nixio__b64encode_tbl[(cv >> 12) & 0x3f];
		*o     = nixio__b64encode_tbl[(cv >> 18) & 0x3f];
		o += 4;
	}

	if (pad) {
		uint32_t cv = data[len-pad] << 16;
		*(o-1) = '=';
		*(o-2) = '=';
		if (pad == 2) {
			cv |= data[len-pad+1] << 8;
			*(o-2) = nixio__b64encode_tbl[(cv >> 6) & 0x3f];
		}
		*(o-3) = nixio__b64encode_tbl[(cv >> 12) & 0x3f];
		*(o-4) = nixio__b64encode_tbl[(cv >> 18) & 0x3f];
	}

	lua_pushlstring(L, out, lenout);
	free(out);
	return 1;
}

static int nixio_bin_b64decode(lua_State *L) {
	size_t len, lenout, i;
	const char *dt = luaL_checklstring(L, 1, &len);

	if (len == 0) {
		lua_pushvalue(L, 1);
		return 1;
	} else if (len % 4) {
		errno = EINVAL;
		return nixio__perror(L);
	}

	lenout = len / 4 * 3;

	unsigned char *out = malloc(lenout);
	if (!out) {
		return luaL_error(L, NIXIO_OOM);
	}

	unsigned char *o = out;
	for (i = 0; i < len; i += 4) {
		uint32_t cv = 0;
		for (int j = 0; j < 4; j++) {
			unsigned char c = dt[i + j] - 43;
			if (c > 79 || (c = nixio__b64decode_tbl[c]) == 0xff) {
				free(out);
				errno = EINVAL;
				return nixio__perror(L);
			}

			cv |= c;
			if (j != 3) {
				cv <<= 6;
			}
		}

		*(o+2) = (unsigned char)(cv & 0xff);
		*(o+1) = (unsigned char)((cv >>  8) & 0xff);
		*o     = (unsigned char)((cv >> 16) & 0xff);
		o += 3;
	}

	if (dt[len-1] == '=') {
		lenout--;
	}

	if (dt[len-2] == '=') {
		lenout--;
	}

	lua_pushlstring(L, (char*)out, lenout);
	free(out);
	return 1;
}

static int nixio_bin_hextonum(lua_State *L) {
	size_t len;
	uint32_t number = 0;
	const char *hex = luaL_checklstring(L, 1, &len);

	if (!((len == 2) | (len == 4) | (len == 8))) {
		errno = EINVAL;
		return nixio__perror(L);
	}

	for (size_t i = 0; i < len; i++) {
		char c = hex[i];
		number <<= 4; /* make room to shift-in next nibble */

		if (c >= '0' && c <= '9') {
			number |= c - '0';
		}
		else if (c >= 'a' && c <= 'f') {
			number |= c - 'a' + 10;
		}
		else if (c >= 'A' && c <= 'F') {
			number |= c - 'A' + 10;
		}
		else {
			errno = EINVAL;
			return nixio__perror(L);
		}

	}

	nixio__pushnumber(L, number);
	return 1;
}

static int nixio_bin_numtohex(lua_State *L) {
	char hex[8];
	uint32_t number = nixio__checknumber(L, 1);
	size_t len = luaL_optinteger(L, 2, 4); /* default 4 bytes hex encoding */

	if (!((len == 1) | (len == 2) | (len == 4))) {
		errno = EINVAL;
		return nixio__perror(L);
	}

	size_t lenout = len * 2;
	for (size_t i = 0; i < lenout; i++) {
		hex[lenout - 1 - i] = nixio__bin2hex[(number & 0x0f)];
		number >>= 4;
	}

	lua_pushlstring(L, hex, lenout);
	return 1;
}

static int nixio_bin_white(lua_State *L) {
	size_t len;
	char *buffer = luaL_checklstring(L, 1, &len);
	uint8_t mod = luaL_optinteger(L, 2, 64);

	for (size_t i=0; i<len; i++) {
		buffer[i] ^= 0xD3 * (uint8_t)(i % mod);
	}

	lua_pushlstring(L, buffer, len);
	return 1;
}

/* module table */
static const luaL_reg R[] = {
	{"hexlify",   nixio_bin_hexlify},
	{"unhexlify", nixio_bin_unhexlify},
	{"crc32",     nixio_bin_crc32},
	{"crc16",     nixio_bin_crc16},
	{"dow_crc",   nixio_bin_dow_crc},
	{"b64encode", nixio_bin_b64encode},
	{"b64decode", nixio_bin_b64decode},
	{"hextonum",  nixio_bin_hextonum},
	{"numtohex",  nixio_bin_numtohex},
	{"white",     nixio_bin_white},
	{NULL,        NULL}
};


void nixio_open_bin(lua_State *L) {
	lua_newtable(L);
	luaL_register(L, NULL, R);
	lua_setfield(L, -2, "bin");
}
