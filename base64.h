#if !defined(H_BASE64)
#define H_BASE64


#include <limits.h>
#include <stddef.h>

#if CHAR_BIT != 8
	#error "CHAR_BIT must be 8 to use this library."
#endif

void	encb64(char *, const void *, size_t);
size_t	encb64len(size_t);

#endif
