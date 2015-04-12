#ifndef MAT_HWDEFS_H
#define MAT_HWDEFS_H

	#define DDR(x) (*(&x - 1))
	#define PIN(x) (*(&x - 2))

#endif
