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

#define PARSER_LOG TU_LOG1

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
      PARSER_LOG("Unknown format size %d\n", size);
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
  PARSER_LOG("####Build event %d #####\n", currentUsage->nb_value);
   switch (currentUsage->type) {
     case BUTTON_PAGE_USAGE:
     {
       int start = currentUsage->begin;
       int stop = currentUsage->end;
       int offset = currentUsage->offset;
       for (int i = start; i<stop; i++) {
         PARSER_LOG("Offset goes to %d\n", offset);
         event->begin = offset;
         event->end = offset+1;
         event->key = i;
         offset += currentUsage->size;
         PARSER_LOG("Generate button %d from %d to %d\n", event->key, event->begin, event->end );
         mapping->nb_events++;
         event = &mapping->events[mapping->nb_events];
       }
     }
     break;
    case BUTTON_USAGE:
    {
        int offset = currentUsage->offset;
        for (int i = 0; i<currentUsage->nb_value; i++) {
          if ((currentUsage->value[i] >= HID_X) && (currentUsage->value[i] <= HID_HAT_SWITCH)) {
            PARSER_LOG("Detected Axis is %x\n", currentUsage->value[i]);
            bool replaced = false;
            for (int j = 0; j<mapping->nb_events; j++) {
              hid_event* testevent = &mapping->events[j];
              if (testevent->key == currentUsage->value[i]) {
                event = testevent;
                replaced = true;
              }
            }
            //Last axis is the good one
            event->begin = offset;
            event->end = offset+currentUsage->size;
            event->shift = currentUsage->size - 8; //3DO expect 0..255 values
            event->key = currentUsage->value[i];
            if (!replaced) mapping->nb_events++;
            event = &mapping->events[mapping->nb_events];
          } else {
            if (currentUsage->value[i] == 0x0)
            PARSER_LOG("Undefined usage\n");
            else
            PARSER_LOG("Unknown Usage (0x%x)\n", currentUsage->value[i]);
          }
          offset += currentUsage->size;
        }
        currentUsage->nb_value = 0;
    }
    break;
    default:
    break;
  }
  currentUsage->offset += currentUsage->size * currentUsage->count;
  PARSER_LOG("######### Offset is %d\n", currentUsage->offset);
  currentUsage->type = UNKNOWN_USAGE;
}

int parse_hid_descriptor(uint8_t const* desc_report, uint16_t desc_len, hid_controller* ctrl)
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
  int level = 0;
  int currentAxisSize[HID_HAT_SWITCH-HID_X+1];
  input_type currentType;

  hid_mapping *mapping = &ctrl->mapping[ctrl->nb_HID];

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
        {
          usage *prev = currentUsage->prev;
          prev->offset = currentUsage->offset;
          prev->next = NULL;
          free(currentUsage);
          currentUsage = prev;
          PARSER_LOG("Usage Pop\n");
        }
        break;
        case HID_PUSH_TAG:
        {
          usage *next = (usage*)calloc(1, sizeof(usage));
          next->offset = currentUsage->offset;
          currentUsage->next = next;
          next->prev = currentUsage;
          currentUsage=next;
          PARSER_LOG("Usage Push\n");
        }
        break;
        case HID_END_COLLECTION_TAG:
          PARSER_LOG("Usage End Collection #### %d ####\n", level);
          level--;
          if (level == 0) {
            ctrl->nb_HID++;
            mapping = &ctrl->mapping[ctrl->nb_HID];
            memset(currentUsage, 0, sizeof(usage));
          }
          break;
        default:
          PARSER_LOG("unknown tag 0x%x with null size\n", tag);
      }
    } else {
      val = read_from_desc(&desc_report[idx],size);
      idx+=size;
      switch(tag) {
        case HID_USAGE_TAG:
          switch(val) {
            case HID_GENERIC_DESKTOP_CONTROLS:
              PARSER_LOG("Generic Desktop controls\n");
              break;
            case HID_JOYSTICK_USAGE:
              currentType = TYPE_JOYSTICK;
              PARSER_LOG("Joystick HID\n");
              break;
            case HID_JOYPAD_USAGE:
              currentType = TYPE_JOYPAD;
              PARSER_LOG("Joypad HID\n");
              break;
            case HID_MOUSE_USAGE:
              currentType = TYPE_MOUSE;
              PARSER_LOG("Mouse HID\n");
              break;
            case HID_BUTTON_PAGE:
              PARSER_LOG("Got a button interface %d buttons\n", currentUsage->end-currentUsage->begin);
              currentUsage->type = BUTTON_PAGE_USAGE;
              break;
            case HID_VENDOR_PAGE:
              PARSER_LOG("Vendor Page\n", nb);
              break;
            default:
              if (type == HID_TYPE_LOCAL) {
                // axis
                  PARSER_LOG("axis 0x%x\n", val);
                  currentUsage->type = BUTTON_USAGE;
                  currentUsage->value[currentUsage->nb_value++] = val;
              } else {
                PARSER_LOG("Unknow usage tag 0x%x\n", val);
              }
          }
        break;
        case HID_COLLECTION_TAG:
          PARSER_LOG("Collection (%s) level %d\n", (val==HID_APPLICATION_COLLECTION)?"Application":(val==HID_LOGICAL_COLLECTION)?"Logical":"unknown", level);
          level++;
          break;
        case HID_REPORT_COUNT_TAG:
          if (type == HID_TYPE_GLOBAL) {
            PARSER_LOG("report Count %d\n", val);
            currentUsage->count = val;
          } else {
            PARSER_LOG("Input (Sata, Var, Abs) 0x%x\n", val);
            addUsage(currentUsage, mapping);
          }
          break;
        case HID_REPORT_SIZE_TAG:
          PARSER_LOG("report Size %d\n", val);
          currentUsage->size = val;
          break;
        case HID_LOGICAL_MINIMUM_TAG:
          if (type == HID_TYPE_GLOBAL)
            PARSER_LOG("logical min %d\n", val);
          if (type == HID_TYPE_LOCAL) {
            PARSER_LOG("usage min %d\n", val);
            currentUsage->begin = val-1;
          }
          break;
        case HID_LOGICAL_MAXIMUM_TAG:
          if (type == HID_TYPE_GLOBAL)
            PARSER_LOG("logical max %d\n", val);
          if (type == HID_TYPE_LOCAL) {
            PARSER_LOG("usage max %d\n", val);
            currentUsage->end = val;
          }
          break;
        case HID_PHYSICAL_MINIMUM_TAG:
          PARSER_LOG("physical min %d\n", val);
          break;
        case HID_PHYSICAL_MAXIMUM_TAG:
          PARSER_LOG("physical max %d\n", val);
          break;
        case HID_INPUT_TAG:
          if (type == HID_TYPE_GLOBAL) {
            PARSER_LOG("Report Id 0x%x\n", val);
            mapping->index = val;
            ctrl->has_index = true;
            currentUsage->offset += 8;
          } else {
            PARSER_LOG("Input (Sata, Var, Abs) 0x%x\n", val);
            mapping->type = currentType; //Type is only valid if there was at least an input in the report
            addUsage(currentUsage, mapping);
          }
          break;
        case HID_UNIT_TAG:
          PARSER_LOG("Unit 0x%x\n", val);
          break;
        default:
          PARSER_LOG("unknown tag 0x%x with val 0x%x\n", tag, val);
      }
    }
  }
  return idx;
}