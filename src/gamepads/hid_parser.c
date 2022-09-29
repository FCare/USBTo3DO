/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021, Ha Thach (tinyusb.org)
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
 *
 */

#include "bsp/board.h"
#include "tusb.h"

#include <stdlib.h>

#include "3DO.h"
#include "hid_parser.h"

hid_mapping currentController;

static int read_from_desc(uint8_t const* desc_report, uint8_t size) {
  switch (size) {
    case 1:
      return desc_report[0];
    case 2:
      return tu_le16toh(tu_unaligned_read16(desc_report));
    case 4:
      return tu_unaligned_read32(desc_report);
    default:
      TU_LOG1("Unknown format size %d\n", size);
  }
  return 0;
}

static bool get_header(uint8_t const* desc_report, uint8_t *tag, uint8_t *type, uint8_t* idx) {
  *tag = (desc_report[0]>>4) & 0xF;
  *type = (desc_report[0]>>2)&0x3;
  *idx = desc_report[0]&0x3;
  return true;
}

static void addUsage(usage *currentUsage, hid_mapping* mapping) {
  hid_event* event = &mapping->events[mapping->nb_events];
  TU_LOG1("####Build event #####\n");
   switch (currentUsage->type) {
     case BUTTON_PAGE_USAGE:
     {
       int start = currentUsage->begin;
       int stop = currentUsage->end;
       for (int i = start; i<stop; i++) {
         TU_LOG1("Offset goes to %d\n", currentUsage->offset);
         event->begin = currentUsage->offset;
         event->end = currentUsage->offset+1;
         event->key = i;
         currentUsage->offset += currentUsage->size;
         TU_LOG1("Generate button %d from %d to %d\n", event->key, event->begin, event->end );
         mapping->nb_events++;
         event = &mapping->events[mapping->nb_events];
       }
     }
     break;
    case BUTTON_USAGE:
    {
        currentUsage->offset += currentUsage->size;
        if ((currentUsage->value >= HID_X) && (currentUsage->value <= HID_HAT_SWITCH)) {
          TU_LOG1("Detected Axis is %x\n", currentUsage->value);
        } else {
          if (currentUsage->value == 0x0)
            TU_LOG1("Undefined usage\n");
          else
            TU_LOG1("Unknown Usage (0x%x)\n", currentUsage->value);
        }
    }
    break;
    default:
    break;
  }
  TU_LOG1("######### Offset is %d\n", currentUsage->offset);
  currentUsage->type = UNKNOWN_USAGE;
}

void parse_hid_descriptor(uint8_t const* desc_report, uint16_t desc_len, hid_mapping* mapping)
{
  uint idx = 0;
  uint8_t size = 0;
  uint8_t type;
  uint8_t tag;
  uint val;
  uint min_local = 0;
  uint max_local = 0;
  uint min = 0;
  uint max = 0;
  uint nb = 0;
  int axis = 0;
  int currentAxisSize[HID_HAT_SWITCH-HID_X+1];

  usage HIDUsage = {0};

  usage *currentUsage = &HIDUsage;

  mapping->nb_events = 0;
  memset(mapping->events, 0, sizeof(mapping->events));
  mapping->type = TYPE_NONE;
  while ((idx < desc_len) && get_header(&desc_report[idx], &tag, &type, &size)) {
    idx++;
    if (size == 0) {
      switch(tag) {
        case HID_POP_TAG:
          addUsage(currentUsage, mapping);
          usage *prev = currentUsage->prev;
          prev->offset = currentUsage->offset;
          prev->next = NULL;
          free(currentUsage);
          currentUsage = prev;
          TU_LOG1("Usage Pop\n");
        break;
        case HID_PUSH_TAG:{
          usage *next = (usage*)calloc(1, sizeof(usage));
          next->offset = currentUsage->offset;
          currentUsage->next = next;
          next->prev = currentUsage;
          currentUsage=next;
          TU_LOG1("Usage Push\n");
        }
        break;
        case HID_END_COLLECTION_TAG:
          addUsage(currentUsage, mapping);
          TU_LOG1("Usage End Collection\n");
          break;
        default:
          TU_LOG1("unknown tag 0x%x with null size\n", tag);
      }
    } else {
      val = read_from_desc(&desc_report[idx],size);
      idx+=size;
      switch(tag) {
        case HID_USAGE_TAG:
          switch(val) {
            case HID_GENERIC_DESKTOP_CONTROLS:
              TU_LOG1("Generic Desktop controls\n");
              break;
            case HID_JOYSTICK_USAGE:
              mapping->type = TYPE_JOYSTICK;
              TU_LOG1("Joystick HID\n");
              break;
            case HID_JOYPAD_USAGE:
              mapping->type = TYPE_JOYPAD;
              TU_LOG1("Joypad HID\n");
              break;
            case HID_MOUSE_USAGE:
              mapping->type = TYPE_MOUSE;
              TU_LOG1("Mouse HID\n");
              break;
            case HID_BUTTON_PAGE:
              TU_LOG1("Got a button interface %d buttons\n", currentUsage->end-currentUsage->begin);
              currentUsage->type = BUTTON_PAGE_USAGE;
              break;
            case HID_VENDOR_PAGE:
              TU_LOG1("Vendor Page\n", nb);
              break;
            default:
              if (type == HID_TYPE_LOCAL) {
                // axis
                  TU_LOG1("axis 0x%x\n", val);
                  currentUsage->type = BUTTON_USAGE;
                  currentUsage->value = val;
                  addUsage(currentUsage, mapping);
              } else {
                TU_LOG1("Unknow usage tag 0x%x\n", val);
              }
          }
        break;
        case HID_COLLECTION_TAG:
          TU_LOG1("Got a %s collection\n", (val==HID_APPLICATION_COLLECTION)?"Application":(val==HID_LOGICAL_COLLECTION)?"Logical":"unknown");
          break;
        case HID_REPORT_COUNT_TAG:
          TU_LOG1("report Count %d\n", val);
          currentUsage->count = val;
          break;
        case HID_REPORT_SIZE_TAG:
          TU_LOG1("report Size %d\n", val);
          currentUsage->size = val;
          break;
        case HID_LOGICAL_MINIMUM_TAG:
          if (type == HID_TYPE_GLOBAL)
            TU_LOG1("logical min %d\n", val);
          if (type == HID_TYPE_LOCAL) {
            TU_LOG1("usage min %d\n", val);
            currentUsage->begin = val-1;
          }
          break;
        case HID_LOGICAL_MAXIMUM_TAG:
          if (type == HID_TYPE_GLOBAL)
            TU_LOG1("logical max %d\n", val);
          if (type == HID_TYPE_LOCAL) {
            TU_LOG1("usage max %d\n", val);
            currentUsage->end = val;
          }
          break;
        case HID_PHYSICAL_MINIMUM_TAG:
          TU_LOG1("physical min %d\n", val);
          break;
        case HID_PHYSICAL_MAXIMUM_TAG:
          TU_LOG1("physical max %d\n", val);
          break;
        case HID_INPUT_TAG:
          TU_LOG1("Input (Sata, Var, Abs) 0x%x\n", val);
          break;
        case HID_UNIT_TAG:
          TU_LOG1("Unit 0x%x\n", val);
          break;
        default:
          TU_LOG1("unknown tag 0x%x with val 0x%x\n", tag, val);
      }
    }
  }
  addUsage(currentUsage, mapping);
}