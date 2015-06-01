/*******************************************************************************
 * T3 Keyboard v1.0
 *
 * Copyright 2014 Chris Nucci (t3@fourbyte.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. 
 ******************************************************************************/

#ifndef T3_WINDOW_H
#define T3_WINDOW_H

#include <pebble.h>

/**
 * Whether diagnostic information of keyboard events should be logged.
 */
#define T3_LOGGING 0

/**
 * Whether to build the pre-defined lower-case keyboard into the app.
 * It is recommended you set this to 0 if you are not using it.
 */
#define T3_INCLUDE_LAYOUT_LOWERCASE 1
	
/**
 * Whether to build the pre-defined upper-case keyboard into the app.
 * It is recommended you set this to 0 if you are not using it.
 */
#define T3_INCLUDE_LAYOUT_UPPERCASE 1
	
/**
 * Whether to build the pre-defined number keyboard into the app.
 * It is recommended you set this to 0 if you are not using it.
 */
#define T3_INCLUDE_LAYOUT_NUMBERS 1
	
/**
 * Whether to build the pre-defined punctuation keyboard into the app.
 * It is recommended you set this to 0 if you are not using it.
 */
#define T3_INCLUDE_LAYOUT_PUNC 1
	
/**
 * Whether to build the pre-defined bracket keyboard into the app.
 * It is recommended you set this to 0 if you are not using it.
 */
#define T3_INCLUDE_LAYOUT_BRACKETS 1

/**
 * A pre-defined keyboard with lower-case letters and a space:
 *   abc  def  ghi
 *   jkl  mno  pqr
 *   stu  vwx  yz 
 */
#if T3_INCLUDE_LAYOUT_LOWERCASE
extern const char T3_LAYOUT_LOWERCASE[];
#endif

/**
 * A pre-defined keyboard with upper-case letters and a space:
 *   ABC  DEF  GHI
 *   JKL  MNO  PQR
 *   STU  VWX  YZ 
 */
#if T3_INCLUDE_LAYOUT_UPPERCASE
extern const char T3_LAYOUT_UPPERCASE[];
#endif

/**
 * A pre-defined keyboard with numbers:
 *   01   2    3
 *   4    5    6
 *   7    8    9
 */
#if T3_INCLUDE_LAYOUT_NUMBERS
extern const char T3_LAYOUT_NUMBERS[];
#endif

/**
 * A pre-defined keyboard with punctuation, operators, etc.:
 *   .    '!   :;"
 *   ,    -    @$#
 *   ?    &%   +*=
 */
#if T3_INCLUDE_LAYOUT_PUNC
extern const char T3_LAYOUT_PUNC[];
#endif

/**
 * A pre-defined keyboard with brackets, slashes, and other
 * miscellaneous characters:
 *   ()   <>   {}
 *   /    \    []
 *   |_   ~^`  ¢½
 */
#if T3_INCLUDE_LAYOUT_BRACKETS
extern const char T3_LAYOUT_BRACKETS[];
#endif

#if PBL_COLOR
/**
  * Sets a pre-defined gray color theme to the window.
  */
#define T3_SET_THEME_GRAY(t3window) t3window_set_colors(t3window, GColorDarkGray, GColorLightGray, GColorBlack, GColorWhite, GColorBlack, GColorWhite, GColorBlack, GColorLightGray, GColorBlack)
/**
  * Sets a pre-defined blue color theme to the window.
  */
#define T3_SET_THEME_BLUE(t3window) t3window_set_colors(t3window, GColorPictonBlue, GColorBlueMoon, GColorWhite, GColorElectricBlue, GColorDukeBlue, GColorWhite, GColorBlack, GColorElectricBlue, GColorDukeBlue)
/**
  * Sets a pre-defined red color theme to the window.
  */
#define T3_SET_THEME_RED(t3window) t3window_set_colors(t3window, GColorBulgarianRose, GColorDarkCandyAppleRed, GColorWhite, GColorRed, GColorBlack, GColorWhite, GColorBlack, GColorDarkCandyAppleRed, GColorBlack)
/**
  * Sets a pre-defined green color theme to the window.
  */
#define T3_SET_THEME_GREEN(t3window) t3window_set_colors(t3window, GColorDarkGreen, GColorMayGreen, GColorBlack, GColorMintGreen, GColorBlack, GColorWhite, GColorBlack, GColorMintGreen, GColorBlack)
#endif

/**
 * The maximum number of characters that the user may enter.
 */
#define T3_MAXLENGTH 24

/**
 * The T3Window type.
 *
 * This is the element that will be created and should be passed to the
 * interface functions.
 */
typedef struct _t3_T3Window T3Window;

/**
 * The function signature for a handler that fires when the T3Window is popped
 * from the stack by the user. It provides the text that was entered.
 */
typedef void (*T3CloseHandler)(const char * text);

/**
 * Creates a new T3Window, given the keyboard layouts and callback function.
 *
 * A keyboard set contains keyboard layouts. If a set contains multiple layouts,
 * the user will be able to cycle through them by holding the UP, SELECT, or DOWN
 * buttons. At least one keyboard layout must be defined and the first keyboard
 * found, starting with set 1, will be shown by default.
 *
 * Example usage, using all pre-defined keyboard layouts:
 *
 *   const char * keyboardSet1[] = {T3_LAYOUT_LOWERCASE, T3_LAYOUT_UPPERCASE};
 *   const char * keyboardSet2[] = {T3_LAYOUT_NUMBERS};
 *   const char * keyboardSet3[] = {T3_LAYOUT_PUNC, T3_LAYOUT_BRACKETS};
 *
 *   void myCloseHandler(const char * text) {
 *      // Do something
 *   }
 *
 *   T3Window * createT3Window() {
 *      T3Window * myT3Window = t3window_create(
 *         keyboardSet1, 2,
 *	       keyboardSet2, 1,
 *         keyboardSet3, 2,
 *         (T3CloseHandler)myCloseHandler);
 *      return myT3Window;
 *   }
 *
 * A keyboard layout is defined by a 36-character string: Nine key definitions,
 * each key consisting of four characters.  A key may start with up to three
 * printable characters and the rest is filled with null terminators.  Keys are
 * defined in the order of: top-left, top-center, top-right, middle-left,
 * middle-center, middle-right, bottom-left, bottom-center, bottom-right.
 *
 *    layout := key key key key key key key key key
 *    key := null null null null
 *         | char null null null
 *         | char char null null
 *         | char char char null
 *    char := (any printable character)
 *    null := \0
 *
 * @param set1  A pointer to an array of strings containing the keyboard layouts
 *              that the user may cycle through using the UP button.
 *              This may be null.
 * @param count1  The number of keyboard layouts in set1.
 *                This should be 0 if set1 is null.
 * @param set2  A pointer to an array of strings containing the keyboard layouts
 *              that the user may cycle through using the SELECT button.
 *              This may be null.
 * @param count2  The number of keyboard layouts in set2.
 *                This should be 0 if set2 is null.
 * @param set3  A pointer to an array of strings containing the keyboard layouts
 *              that the user may cycle through using the DOWN button.
 *              This may be null.
 * @param count3  The number of keyboard layouts in set3.
 *                This should be 0 if set3 is null.
 * @param closeHandler  The T3CloseHandler to fire when the keyboard closes.
 *                      This may be null.
 * @return A pointer to a new T3Window.
 */
T3Window * t3window_create(const char ** set1, uint8_t count1,
						 const char ** set2, uint8_t count2,
						 const char ** set3, uint8_t count3,
						 T3CloseHandler closeHandler);


#if PBL_COLOR
/**
 * Sets the window colors if color is supported.
 * Pressed key colors are inferred, but they may be overridden
 * using t3_window_set_pressed_key_colors().
 * If not called, a default scheme is used.
 *
 * @param window  The T3Window whose colors to set.
 * @param background  The background color of the window.
 * @param keyFace  The primary color of the keys.
 * @param keyText  The color of the key text.
 * @param keyHighlight  The highlight color of the keys.
 * @param keyShadow  The shadow color of the keys.
 * @param editBackground  The background color of the edit area.
 * @param editText  The color of the edit area text.
 * @param editHighlight  The highlight color of the edit area.
 * @param editShadow  The shadow color of the edit area.
 */
void t3window_set_colors(T3Window * window, GColor background,
						 GColor keyFace, GColor keyText,
						 GColor keyHighlight, GColor keyShadow,
						 GColor editBackground, GColor editText,
						 GColor editHighlight, GColor editShadow);

/**
 * Overrides the colors used for a key that is pressed.
 * Note that a pressed key is presented as sunken,
 * so the highlight is on the bottom and right sides
 * and the shadow is on the top and left sides.
 *
 * @param window  The T3Window whose pressed key colors to set.
 * @param keyFace  The primary color of the pressed key.
 * @param keyText  The color of the pressed key text.
 * @param keyHighlight  The highlight color of the pressed key.
 * @param keyShadow  The shadow color of the pressed key.
 */
void t3window_set_pressed_key_colors(T3Window * window,
									GColor keyFace, GColor keyText,
									GColor keyHighlight, GColor keyShadow);
#endif

/**
 * Destroys a T3Window previously created by t3window_create().
 *
 * @param window  The T3Window to destroy.
 */
void t3window_destroy(T3Window * window);

/**
 * Places the T3Window on the window stack.
 *
 * This call is equivalent to:
 *   window_stack_push(myT3Window->window, animated);
 *
 * @param window  The T3Window to display.
 * @param animated  Whether to use the window_stack_push() animation.
 */
void t3window_show(const T3Window * window, bool animated);

/**
 * Sets the input text in the T3Window.
 *
 * @param window  The T3Window whose text to set.
 * @param text  A pointer to the text to display.
 *              This will be copied locally, up to the T3_MAXLENGTH,
 *              so a stack-allocated string may be used.
 */
void t3window_set_text(T3Window * window, const char * text);

/**
 * Gets the input text from the T3Window.
 *
 * @param window  The T3Window whose text to get.
 * @return  A pointer to the text displayed in the window.
 */
const char * t3window_get_text(const T3Window * window);

#endif