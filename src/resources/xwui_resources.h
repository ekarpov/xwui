/*
 *  XWUI in-memory resources
 */

#ifndef _XWUI_RESOURCES_H_
#define _XWUI_RESOURCES_H_

/*----------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif 

/*----------------------------------------------------------------------*/
/* helper macro */
#define XWUI_DEFINE_BITMAP_DATA(_name, _ext) \
        extern const unsigned char (##_name##_##_ext##_##data[]);\
        extern const size_t (##_name##_##_ext##_##data##_##size);

/*----------------------------------------------------------------------*/
/* bitmaps */

/* spinner frames 26px */
XWUI_DEFINE_BITMAP_DATA(spinner_frame_1_26, png);
XWUI_DEFINE_BITMAP_DATA(spinner_frame_2_26, png);
XWUI_DEFINE_BITMAP_DATA(spinner_frame_3_26, png);
XWUI_DEFINE_BITMAP_DATA(spinner_frame_4_26, png);
XWUI_DEFINE_BITMAP_DATA(spinner_frame_5_26, png);
XWUI_DEFINE_BITMAP_DATA(spinner_frame_6_26, png);
XWUI_DEFINE_BITMAP_DATA(spinner_frame_7_26, png);
XWUI_DEFINE_BITMAP_DATA(spinner_frame_8_26, png);

/* spinner frames 32px */
XWUI_DEFINE_BITMAP_DATA(spinner_frame_1_32, png);
XWUI_DEFINE_BITMAP_DATA(spinner_frame_2_32, png);
XWUI_DEFINE_BITMAP_DATA(spinner_frame_3_32, png);
XWUI_DEFINE_BITMAP_DATA(spinner_frame_4_32, png);
XWUI_DEFINE_BITMAP_DATA(spinner_frame_5_32, png);
XWUI_DEFINE_BITMAP_DATA(spinner_frame_6_32, png);
XWUI_DEFINE_BITMAP_DATA(spinner_frame_7_32, png);
XWUI_DEFINE_BITMAP_DATA(spinner_frame_8_32, png);

/* bitmap buttons 26px */
XWUI_DEFINE_BITMAP_DATA(add_icon_26, png);
XWUI_DEFINE_BITMAP_DATA(add_selected_icon_26, png);
XWUI_DEFINE_BITMAP_DATA(back_icon_26, png);
XWUI_DEFINE_BITMAP_DATA(back_selected_icon_26, png);
XWUI_DEFINE_BITMAP_DATA(refresh_icon_26, png);
XWUI_DEFINE_BITMAP_DATA(refresh_selected_icon_26, png);
XWUI_DEFINE_BITMAP_DATA(delete_icon_26, png);
XWUI_DEFINE_BITMAP_DATA(delete_selected_icon_26, png);
XWUI_DEFINE_BITMAP_DATA(create_new_icon_26, png);
XWUI_DEFINE_BITMAP_DATA(create_new_selected_icon_26, png);

/* bitmap buttons 15px */
XWUI_DEFINE_BITMAP_DATA(close_15_filled, png);
XWUI_DEFINE_BITMAP_DATA(close_15_filled_transparent, png);
XWUI_DEFINE_BITMAP_DATA(delete_15, png);
XWUI_DEFINE_BITMAP_DATA(delete_filled_15, png);

/*----------------------------------------------------------------------*/
/* strings */

/* en-us */
extern const size_t xwui_strings_en_us_strings_count;
extern const char* xwui_strings_en_us_strings[];
extern const wchar_t* xwui_strings_en_us_values[];

/* ru-ru */
extern const size_t xwui_strings_ru_ru_strings_count;
extern const char* xwui_strings_ru_ru_strings[];
extern const wchar_t* xwui_strings_ru_ru_values[];

/*----------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif 

/*----------------------------------------------------------------------*/

#endif /* _XWUI_RESOURCES_H_ */


