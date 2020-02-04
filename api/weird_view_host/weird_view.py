"""
This file contians weird view protocol defintions.
"""

# Configuration options
WV_REQ_TIMEOUT          = 0.5

# Weird view protocol definitions.
WV_DISC                 = "8737808B"
WV_DISC_REPLY           = "134E3A9D"
WV_LIST                 = "7DE0B79C"
WV_LIST_REPLY           = "57186F8A"
WV_UPDATE               = "13AC8F62"
WV_UPDATE_REPLY         = "0F4E180A"
WV_REQ                  = "81c4b463"

# Weird view plugin definitions.
WV_PLUGIN_LOG           = '0'
WV_PLUGIN_SWITCH        = '1'
WV_PLUGIN_ANALOG        = '2'
WV_PLUGIN_WAVE          = '3'

# Wierd view log plugin definitions.
WV_PLUGIN_LOG_UPDATE    = 0x0
WV_PLUGIN_LOG_APPEND    = 0x1

# Wierd view switch plugin definitions.
WV_PLUGIN_SWITCH_OFF    = 0x0
WV_PLUGIN_SWITCH_ON     = 0x1
