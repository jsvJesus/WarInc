#if defined(_MSC_VER)

#if defined(_M_IX86)

#pragma comment(linker, "/alternatename:?jpeg_start_decompress@@YAEPAUjpeg_decompress_struct@@@Z=_jpeg_start_decompress")
#pragma comment(linker, "/alternatename:?jpeg_read_header@@YAHPAUjpeg_decompress_struct@@E@Z=_jpeg_read_header")
#pragma comment(linker, "/alternatename:?jpeg_destroy_decompress@@YAXPAUjpeg_decompress_struct@@@Z=_jpeg_destroy_decompress")
#pragma comment(linker, "/alternatename:?jpeg_CreateDecompress@@YAXPAUjpeg_decompress_struct@@HI@Z=_jpeg_CreateDecompress")
#pragma comment(linker, "/alternatename:?jpeg_finish_decompress@@YAEPAUjpeg_decompress_struct@@@Z=_jpeg_finish_decompress")
#pragma comment(linker, "/alternatename:?jpeg_abort_decompress@@YAXPAUjpeg_decompress_struct@@@Z=_jpeg_abort_decompress")
#pragma comment(linker, "/alternatename:?jpeg_read_scanlines@@YAIPAUjpeg_decompress_struct@@PAPAEI@Z=_jpeg_read_scanlines")
#pragma comment(linker, "/alternatename:?jpeg_read_coefficients@@YAPAPAUjvirt_barray_control@@PAUjpeg_decompress_struct@@@Z=_jpeg_read_coefficients")
#pragma comment(linker, "/alternatename:?jpeg_resync_to_restart@@YAEPAUjpeg_decompress_struct@@H@Z=_jpeg_resync_to_restart")
#pragma comment(linker, "/alternatename:?jpeg_std_error@@YAPAUjpeg_error_mgr@@PAU1@@Z=_jpeg_std_error")

#elif defined(_M_X64)

#pragma comment(linker, "/alternatename:?jpeg_start_decompress@@YAEPEAUjpeg_decompress_struct@@@Z=jpeg_start_decompress")
#pragma comment(linker, "/alternatename:?jpeg_read_header@@YAHPEAUjpeg_decompress_struct@@E@Z=jpeg_read_header")
#pragma comment(linker, "/alternatename:?jpeg_destroy_decompress@@YAXPEAUjpeg_decompress_struct@@@Z=jpeg_destroy_decompress")
#pragma comment(linker, "/alternatename:?jpeg_CreateDecompress@@YAXPEAUjpeg_decompress_struct@@HI@Z=jpeg_CreateDecompress")
#pragma comment(linker, "/alternatename:?jpeg_finish_decompress@@YAEPEAUjpeg_decompress_struct@@@Z=jpeg_finish_decompress")
#pragma comment(linker, "/alternatename:?jpeg_abort_decompress@@YAXPEAUjpeg_decompress_struct@@@Z=jpeg_abort_decompress")
#pragma comment(linker, "/alternatename:?jpeg_read_scanlines@@YAIPEAUjpeg_decompress_struct@@PEAPEAEI@Z=jpeg_read_scanlines")
#pragma comment(linker, "/alternatename:?jpeg_read_coefficients@@YAPEAPEAUjvirt_barray_control@@PEAUjpeg_decompress_struct@@@Z=jpeg_read_coefficients")
#pragma comment(linker, "/alternatename:?jpeg_resync_to_restart@@YAEPEAUjpeg_decompress_struct@@H@Z=jpeg_resync_to_restart")
#pragma comment(linker, "/alternatename:?jpeg_std_error@@YAPEAUjpeg_error_mgr@@PEAU1@@Z=jpeg_std_error")

#endif

#endif