/*
 * Copyright (c) 2021 Guilherme Janczak
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * All of the functions in this file are intended to comply to the base64
 * format described in: https://tools.ietf.org/html/rfc2045#section-6.8
 */

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

static const char b64set[64] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

enum {
	MAXLINE		= 76,	/* Maximum base64 line length. */
	GROUPLEN	= 3,	/* Group of bytes to convert to base64. */
	WORDLEN		= 4,	/* Base64 is composed of 4 byte words. */
	B64_BIT		= 6,	/* How many bits are in a base64 character. */
};

/* encb64: encodes _src into base64 on dst.
 *
 * dst must be at least as large as b64len(len)
 * dst and _src cannot overlap.
 */
void
encb64(char *dst, const void *_src, size_t len)
{
	int linelen;
	/* Holds up to GROUPLEN bytes to be encoded in base64. */
	uint32_t group;
	/* How many bytes are left over after aligning to GROUPLEN. */
	int leftover;
	/* alignlen is <= len and aligned to GROUPLEN. */
	size_t alignlen;
	size_t i, j, k;
	const unsigned char *src = _src;

	linelen = group = 0;
	leftover = len % WORDLEN;
	alignlen = len - leftover;
	for (i = j = 0; i < alignlen;) { /* Encode aligned groups. */
		for (k = 0; k < GROUPLEN; k++)
			group |= src[i++] << k * CHAR_BIT;
		for (k = 0; k < WORDLEN; k++) {
			dst[j++] = b64set[group & 0x3f];
			group >>= B64_BIT;
		}
		linelen += WORDLEN;
		if (linelen == MAXLINE) {
			linelen = 0;
			dst[j++] = '\r';
			dst[j++] = '\n';
		}
	}

	/* Grab 0, 1 or 2 leftover unencoded bytes. */
	for (k = 0; k < leftover; k++)
		group |= src[i++] << k * CHAR_BIT;
	/* Convert leftover bytes and count how many conversions were made. */
	for (k = 0; group; k++) {
		dst[j++] = b64set[group & 0x3f];
		group >>= B64_BIT;
	}
	/* Pad with equal signs until the output is 4-byte aligned. */
	if (k) {
		do {
			dst[j++] = '=';
			k++;
		} while (k < WORDLEN);
	}
	/* Terminating \r\n if the output doesn't already have it. */
	if (linelen) {
		dst[j++] = '\r';
		dst[j] = '\n';
	}
}

/* encb64len: return the size of count chars when converted to base64.
 * The answer is always the same for the same count.
 */
size_t
encb64len(size_t count)
{
	size_t b64len;
	b64len = count / GROUPLEN * WORDLEN;
	/* Equal sign padding and/or incomplete word. */
	if (count % GROUPLEN)
		b64len += WORDLEN;
	/* \r\n sequences. */
	b64len += (b64len / MAXLINE) * 2;
	/* Terminating \r\n. */
	b64len += !!(b64len % MAXLINE) * 2;

	return (b64len);
}
