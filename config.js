  'use strict';
/* eslint-disable quotes */

module.exports = [
  { 
    "type": "heading", 
    "defaultValue": "Settings" 
  }, 
  { 
    "type": "text", 
    "defaultValue": "These are the current settings" 
  },
  {
  "type": "section",
  "items": [
    {
      "type": "heading",
      "defaultValue": "settings"
    },
    {
    "type": "color",
      "id": "background",
      "appKey": "background",
      "defaultValue": "00FFAA",
      "label": "Background Color",
      "sunlight": true
    }

    ]
  },
{
  "type": "submit",
  "defaultValue": "Save"
}

];
