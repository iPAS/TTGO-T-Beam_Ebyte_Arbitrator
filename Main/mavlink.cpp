/**
 * @author Pasakorn Tiwatthanont (iPAS)
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Renzo Mischianti www.mischianti.org All right reserved.
 *
 * You may copy, alter and reuse this code in any way you like, but please leave
 * reference to www.mischianti.org in your comments if you redistribute this code.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "global.h"


/**
 * @brief
 *
 */
char * mavlink_segmentor(char * data, size_t len, size_t *new_len) {
    *new_len = len;
    return data;  // XXX: bypass
/*
    static char buffer[MAVLINK_BUFFER_SIZE];
    static size_t buf_len = 0;

    if (buf_len > 0) {  // Concatenate with the old fregment.
        // data = buf + data;
        buf_len = 0;
    }

    const char MAV1_STX = 0xFE;
    const char MAV2_STX = 0xFD;
    char stx = data.charAt(0);

    // Something is wrong ?!
    if (stx != MAV1_STX  &&  stx != MAV2_STX) {
        if (system_verbose_level >= VERBOSE_ERROR) {
            term_printf("[EBYTE] First message is not a Mavlink!" ENDL);
        }

    }

    // A Mavlink message has been found
    else {
        size_t last_index = data.lastIndexOf(stx);
        size_t last_frame_len = (size_t)data.charAt(last_index + 1);
        switch (stx) {
            case MAV1_STX:
                last_frame_len += 2  // stx, payload_len
                                + 3  // seq, sysid, compid
                                + 1  // msgid
                                + 2; // chksum
                break;
            case MAV2_STX:
                last_frame_len += 2  // stx, payload_len
                                + 2  // incomp_flags, comp_flags
                                + 3  // seq, sysid, compid
                                + 3  // msgid
                                + 2  // chksum
                                + 13;// signature
                break;
        }
        size_t complete_frames_len = last_index + last_frame_len;

        /////////////////////
        // Complete frames //
        /////////////////////
        if (complete_frames_len == data.length()) {
            if (system_verbose_level >= VERBOSE_DEBUG) {
                term_printf("[EBYTE] Complete Mavlink messages received, "
                    "STX:%2X len:%lu lst_idx:%lu lst_len:%lu" ENDL,
                    stx, data.length(), last_index, last_frame_len);
            }

        } else
        /////////////////////////////////
        // Data over on the last frame //
        /////////////////////////////////
        if (complete_frames_len < data.length()) {
            if (system_verbose_level >= VERBOSE_DEBUG) {
                term_printf("[EBYTE] Drop a fregment of Mavlink message, "
                    "STX:%2X len:%lu lst_idx:%lu lst_len:%lu" ENDL,
                    stx, data.length(), last_index, last_frame_len);
            }
            data = data.substring(0, complete_frames_len);  // Use only full frames. Drop the left

        } else
        ///////////////////////////////////////////////
        // Lack of data, leave the last frame in buf //
        ///////////////////////////////////////////////
        if (complete_frames_len > data.length()) {
            if (system_verbose_level >= VERBOSE_DEBUG) {
                term_printf("[EBYTE] Leave a fregment of Mavlink message in buf, "
                    "STX:%2X len:%lu lst_idx:%lu lst_len:%lu" ENDL,
                    stx, data.length(), last_index, last_frame_len);
            }
            buf = data.substring(last_index);      // Keep the fragment.
            data = data.substring(0, last_index);  // Use only the full frames.
        }
    }

    *new_len = data.length();
    return (char *)data.c_str();
*/
}


#ifdef TEST_MAVLINK
/**
 * @brief
 *
 */
void mavlink_test_segmmentor() {
    // char *p;
    // size_t len;

    // String data = "";

    // p = mavlink_segmentor(data, &len);

}
#endif
