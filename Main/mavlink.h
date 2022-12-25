#ifndef __MAVLINK_H__
#define __MAVLINK_H__


#define TEST_MAVLINK
#define MAVLINK_BUFFER_SIZE 1024

extern char * mavlink_segmentor(char * data, size_t len, size_t *new_len);

#ifdef TEST_MAVLINK
extern void mavlink_test_segmmentor();
#endif


#endif  // __MAVLINK_H__
