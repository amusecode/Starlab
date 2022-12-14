
////  make_image:  Convert the input stream into an 8-bit color image.
////               Assume that the numbers are *already* in the range [0,255],
////               and are arranged left to right, top to bottom in the frame.
////
////               Use gifsicle or the ImageMagick "convert" program to convert
////               the image to other formats.
////
////  Options:     -c         expect char input
////               -C         attach a comment to a GIF file               [no]
////               -f         expect float input                      [default]
////               -g         same as -F 1
////               -i         expect int input
////               -F fmt     specify image format (gif, png, sun)        [gif]
////               -m file    specify colormap file              [use built-in]
////               -p         same as -F 2
////               -s nx ny   specify image dimensions     [no default;
////                                                        ny = nx if not set]

#include "stdinc.h"
#include "image_fmt.h"

#define USAGE \
"Usage: make_image [-c] [-C comment] [-f] [-i] [-F fmt] [-g] [-p] \n\
       [-m color-map] [-s sizex [sizey]]"

#define BUFSIZE 8192

main(int argc, char** argv)
{
    // Variables that may be set from the command line:

    int m = 0, n = 0;		// image dimensions (no default)
    int which = 2;		// 0 ==> char, 1 ==> int, 2 ==> float [def]
    char* colormap = NULL;	// use standard built-in colormap by default

    unsigned char u;
    int i;
    float f;

    const char *gif = "gif", *png = "png";
    char fmt[8];
    strcpy(fmt, gif);
    bool nset = false;

    char *comment = NULL;

    /* Parse the argument list. */

    for (i = 0; i < argc; i++)
	if (argv[i][0] == '-') {
	    switch (argv[i][1]) {
		case 'C':	comment = argv[++i];
 				break;
		case 'c':	which = 0;
				break;
		case 'f':	which = 2;
				break;
		case 'F':	strcpy(fmt, argv[++i]);
				break;
		case 'g':	strcpy(fmt, gif);
				break;
		case 'i':	which = 1;
				break;
		case 'm':	colormap = argv[++i];
				break;
		case 'p':	strcpy(fmt, png);
				break;
		case 's':	m = atoi(argv[++i]);
				if (i+1 < argc && argv[i+1][0] != '-') {
				    n = atoi(argv[++i]);
				    nset = true;
				}
				break;

		default:	cerr << USAGE << endl;
				exit(1);

	    }
	}

    if (m <= 0) {
	fprintf(stderr, "Must specify first image dimension.\n");
	exit(1);
    }

    // Read and store the data.

    unsigned char *data = (unsigned char*)malloc(BUFSIZE*sizeof(unsigned char));

    int count = 0;
    bool status = true;

    while (status) {

	if (which == 0)
	    status = (scanf("%u", &u) != EOF);
	else if (which == 1) {
	    status = (scanf("%d", &i) != EOF);
	    u = (unsigned char)i;
	} else if (which == 2) {
	    status = (scanf("%f", &f) != EOF);
	    u = (unsigned char)f;
	}

	if (status) {
	    if ((count+1)%BUFSIZE == 0)
		data = (unsigned char*)realloc((void*)data,
			       (count+1+BUFSIZE)*sizeof(unsigned char));
	    data[count++] = u;
	}
    }

    if (nset && count != m*n)
    	fprintf(stderr, "Warning: %d bytes read.\n", count);

    n = count/m;
    count = m*n;		// may truncate the data...

    PRC(n); PRC(m); PRL(count);

    if (streq(fmt, "sun"))
	write_sun(stdout, m, n, data, colormap);

    else if (streq(fmt, "gif"))
	write_gif(stdout, m, n, data, colormap, comment);

    else if (streq(fmt, "png")) {

#ifdef HAVE_LIBPNG
	write_png(stdout, m, n, data, colormap, comment);
#else
	cerr << "No PNG support; switching to GIF" << endl;
	write_gif(stdout, m, n, data, colormap, comment);
#endif
    }
}
