#include "mpoio.h"
#include "ijg_simd/jpeglib.h"
#pragma comment(lib,"ijg6b.lib")

typedef struct {
  struct jpeg_source_mgr pub;	/* public fields */
  JOCTET * buffer;
  unsigned long buffer_length;
} memory_source_mgr;

typedef struct {
  struct jpeg_destination_mgr pub;
  JOCTET * buffer; 
  unsigned long buffer_length;
} memory_destination_mgr;

typedef memory_source_mgr *memory_src_ptr;
typedef memory_destination_mgr *memory_dst_ptr;


METHODDEF(void)
  memory_init_source (j_decompress_ptr cinfo)
{
}

METHODDEF(boolean)
     memory_fill_input_buffer (j_decompress_ptr cinfo)
{
  memory_src_ptr src = (memory_src_ptr) cinfo->src;

  src->buffer[0] = (JOCTET) 0xFF;
  src->buffer[1] = (JOCTET) JPEG_EOI;
  src->pub.next_input_byte = src->buffer;
  src->pub.bytes_in_buffer = 2;
  return TRUE;
}

METHODDEF(void) memory_skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
  memory_src_ptr src = (memory_src_ptr) cinfo->src;
  
  if (num_bytes > 0) {
    src->pub.next_input_byte += (size_t) num_bytes;
    src->pub.bytes_in_buffer -= (size_t) num_bytes;
  }
}

METHODDEF(void) memory_term_source (j_decompress_ptr cinfo)
{
}

GLOBAL(void)
  jpeg_memory_src (j_decompress_ptr cinfo, void* data, unsigned long len)
{
  memory_src_ptr src;
  
  if (cinfo->src == NULL) {	/* first time for this JPEG object? */
    cinfo->src = (struct jpeg_source_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				  sizeof(memory_source_mgr));
    src = (memory_src_ptr) cinfo->src;
    src->buffer = (JOCTET *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				  len * sizeof(JOCTET));
  }
  
  src = (memory_src_ptr) cinfo->src;
  
  src->pub.init_source = memory_init_source;
  src->pub.fill_input_buffer = memory_fill_input_buffer;
  src->pub.skip_input_data = memory_skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
  src->pub.term_source = memory_term_source;
  
  src->pub.bytes_in_buffer = len;
  src->pub.next_input_byte = (JOCTET*)data;
}


/*** RGB -> JPEG on mem ***/

METHODDEF(void) memory_init_destination (j_compress_ptr cinfo){
  
  memory_dst_ptr dest;
  
  dest = (memory_dst_ptr)cinfo->dest;
  
  dest->pub.free_in_buffer = dest->buffer_length;
  
  dest->pub.next_output_byte = dest->buffer;
  
}

METHODDEF(boolean) memory_empty_output_buffer (j_compress_ptr cinfo){
  
  return TRUE;
  
}

METHODDEF(void) memory_term_destination (j_compress_ptr cinfo){
  
}

GLOBAL(void) jpeg_memory_dst (j_compress_ptr cinfo, void* data, unsigned long len){
  
  memory_dst_ptr dest;
  
  if (cinfo->dest == NULL) {/* first time for this JPEG object? */
    
    cinfo->dest = (struct jpeg_destination_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				  sizeof(memory_destination_mgr));
    
  }
  
  dest = (memory_dst_ptr) cinfo->dest;
  
  dest->pub.init_destination = memory_init_destination;
  
  dest->pub.empty_output_buffer = memory_empty_output_buffer;
  
  dest->pub.term_destination = memory_term_destination;
  
  dest->buffer = (JOCTET *)data;
  
  dest->buffer_length = len;
  
}

int jpeg_encode(const unsigned char * const src,
	    const int width, const int height,
	    const unsigned char * const dst,
	    const int quality, const int dct_mode, const bool isOptimize, const int restart_interval)
{
  struct	jpeg_compress_struct	cinfo;
  struct	jpeg_error_mgr	jerr;
  int	y, y_step;
  int ret;

  cinfo.err = jpeg_std_error( &jerr );
  jpeg_create_compress( &cinfo );
  
  jpeg_memory_dst(&cinfo, (void *)dst, width * height * 3);
  //jpeg_stdio_dest( &cinfo, outfile );
  
  cinfo.image_width = width;
  cinfo.image_height = height;
  cinfo.input_components = 3;
  
  cinfo.in_color_space = JCS_RGB;

  jpeg_set_defaults( &cinfo );
  jpeg_set_quality( &cinfo, quality, TRUE );
  if(restart_interval>=0)
	cinfo.restart_interval=restart_interval;

  if(isOptimize)cinfo.optimize_coding=TRUE;
  
 // cinfo.progressive_mode=TRUE;
//	  
  cinfo.dct_method=(J_DCT_METHOD)dct_mode;

  
  int	row_stride;
  JSAMPROW	row_pointer[ 1 ];
  
  jpeg_start_compress( &cinfo, TRUE );
  row_stride = width * 3;
  
  y = 0;
  y_step = 1;
  
  while ( (int)cinfo.next_scanline < height) {
    row_pointer[ 0 ] = (JSAMPLE *)&src[y * width * 3];

    jpeg_write_scanlines( &cinfo, row_pointer, 1 );

    y += y_step;
  }
  
  ret = width * height * 3 - cinfo.dest->free_in_buffer+3;

  jpeg_finish_compress( &cinfo );
  jpeg_destroy_compress( &cinfo );
  
  return ret;
}

int
jpeg_decode( unsigned char * mem_src,		 
	     unsigned char * mem_dst,
	     int width, int height )
{
  struct	jpeg_decompress_struct	cinfo;
  struct	jpeg_error_mgr	jerr;
  int i;

  cinfo.err = jpeg_std_error( &jerr );
  jpeg_create_decompress( &cinfo );
  
  jpeg_memory_src(&cinfo, mem_src, width * height * 3);

  //jpeg_stdio_src( &cinfo, infile );
  
  int	row_stride;
  JSAMPARRAY	row_buffer;
  
  jpeg_read_header( &cinfo, TRUE );
  jpeg_start_decompress( &cinfo );
  row_stride = cinfo.output_width * cinfo.output_components;
  row_buffer = ( *cinfo.mem->alloc_sarray )
    (( j_common_ptr ) &cinfo, JPOOL_IMAGE, row_stride, 1 );
  
  width = cinfo.output_width;
  height = cinfo.output_height;
  //image.resize( width * height * 3 );
  
  int	y, y_step;
  
  y = 0;
  y_step = 1;
  
  
  while ( cinfo.output_scanline < cinfo.output_height ) {
    jpeg_read_scanlines( &cinfo, row_buffer, 1 );
    unsigned char	*src, *dst;
    src = row_buffer[ 0 ];
    dst = &mem_dst[ y * width * 3 ];
    for (i = 0; i < width * 3; i++ ) {
      *dst++ = *src++;
    }
    y += y_step;
  }
  
  jpeg_finish_decompress( &cinfo );
  jpeg_destroy_decompress( &cinfo );
  
  return 0;
}



std::vector<cv::Mat> mporead(char* name, int flags)
{
	std::vector<cv::Mat> ret(2);
	IplImage** a  = cvLoadMPO(name);
	
	if(flags == 1 || a[0]->nChannels==1)
	{
		cv::Mat(a[0]).copyTo(ret[0]);
		cv::Mat(a[1]).copyTo(ret[1]);
	}
	else
	{
		cv::cvtColor(cv::Mat(a[0]),ret[0],CV_BGR2GRAY);
		cv::cvtColor(cv::Mat(a[1]),ret[1],CV_BGR2GRAY);
	}

	cvReleaseMPO(a);

	return ret;
}

void cvReleaseMPO(IplImage** src)
{
	cvReleaseImage(&src[0]);
	cvReleaseImage(&src[1]);
	delete[] src;
}

IplImage** cvLoadMPO(char* name)
{
	
	FILE* fp = fopen(name,"rb");
	fpos_t size = 0;

	/* ファイルサイズを調査 */ 
	fseek(fp,0,SEEK_END); 
	fgetpos(fp,&size); 
	fseek(fp,0,SEEK_SET); 
  
	IplImage* destL = cvLoadImage(name);
	if(destL==NULL)std::cout<<name<<" is invalid file name@loadMPO"<<std::endl;
	IplImage* destR = cvCreateImage(cvGetSize(destL),destL->depth,destL->nChannels);

	int index;
	uchar* src = new uchar[(unsigned int)size];
	fread(src,sizeof(char),(size_t)size,fp);
	for(int i=1;i<size;i++)
	{
		if(src[i] ==0xFF && src[i+1] ==0xD8
			&& src[i+2] ==0xFF && src[i+3] ==0xE1)
		{
			index = i;
			break;
		}
	}
	
	int width = destL->width; 
	int height=destL->height;

	unsigned char * mem_src = src+index;	 
	unsigned char * mem_dst = (uchar*)destR->imageData;
	

	struct	jpeg_decompress_struct	cinfo;
	struct	jpeg_error_mgr	jerr;
	int i;

	cinfo.err = jpeg_std_error( &jerr );
	jpeg_create_decompress( &cinfo );

	
	jpeg_memory_src(&cinfo, mem_src, width * height * 3);
	//jpeg_stdio_src( &cinfo, infile );

	
	int	row_stride;
	JSAMPARRAY	row_buffer;

	jpeg_read_header( &cinfo, TRUE );
	jpeg_start_decompress( &cinfo );
	row_stride = cinfo.output_width * cinfo.output_components;
	row_buffer = ( *cinfo.mem->alloc_sarray )
		(( j_common_ptr ) &cinfo, JPOOL_IMAGE, row_stride, 1 );

	width = cinfo.output_width;
	height = cinfo.output_height;
	//image.resize( width * height * 3 );

	int	y, y_step;

	y = 0;
	y_step = 1;

	while ( cinfo.output_scanline < cinfo.output_height ) {
		jpeg_read_scanlines( &cinfo, row_buffer, 1 );
		unsigned char	*src, *dst;
		src = row_buffer[ 0 ];
		dst = &mem_dst[ y * width * 3 ];
		for (i = 0; i < width * 3; i++ ) {
			*dst++ = *src++;
		}
		y += y_step;
	}

	//
	jpeg_finish_decompress( &cinfo );
	jpeg_destroy_decompress( &cinfo );

	fclose(fp);
	delete[] src;

	cvCvtColor(destR,destR,CV_BGR2RGB);
	IplImage** ret = new IplImage*[2];
	ret[0]=destL;
	ret[1]=destR;
	
	return ret;
}