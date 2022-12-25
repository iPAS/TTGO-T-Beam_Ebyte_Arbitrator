#ifndef __MAVLINK_H__
#define __MAVLINK_H__


#define TEST_MAVLINK


extern char * mavlink_segmentor(String & data, size_t *new_len);

#ifdef TEST_MAVLINK
extern void mavlink_test_segmmentor();
#endif


#endif  // __MAVLINK_H__
