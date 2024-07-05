This repository is the Unreal Engine side of [VCVRVR](https://blog.gotno.net/vcvrvr).

Feel free to file any bugs or feature requests over in [issues](https://github.com/gotno/racksim/issues).

## Things to know
1. You should have the latest version of Rack (> v2.5) installed. RackSim will use your plugins and settings.
2. You **should not** have Rack running before starting RackSim. RackSim needs to manage its own custom version of Rack. It will not work if Rack is already running.
3. Any new plugins or plugin updates will need to be handled by starting your installed Rack and letting it run updates.


## Controls
<a name="far">`far button`</a> (typically `b` or `y`)

<a name="near">`near button`</a> (typically `a` or `x`)

<a name="grip">`grip`</a>

<a name="trigger">`trigger`</a>

<a name="stick">`analog stick`</a>

### Menus
| action | control |
| --- | --- |
| Scroll Menu | [analog stick](#stick) up or down (y-axis) |
| Click Menu | [trigger](#trigger) |

### Base Actions
| action | control |
| --- | --- |
| Summon Library | tap [near](#near) |
| Toggle Menu | tap [far](#far) |
| Save Screenshot | hold [far](#far) |

### World Manipulation
| action | control |
| --- | --- |
| Translate World | hold [grip](#grip) |
| Rotate World | hold [grip](#grip) and [near](#near) on same controller |
| Roto-translate World | hold [grip](#grip) on both controllers |
| Teleport | hold [grip](#grip) and push [analog stick](#stick) of same controller forward to aim, release [analog stick](#stick) to teleport |

### Module Manipulation
| action | control |
| --- | --- |
| Grab Module | hold [grip](#grip) |
| Duplicate Module (initialized) | tap [near](#near) while grabbing |
| Destroy Module | tap [far](#far) while grabbing (once to prime, again to destroy) |
| Toggle Context Menu | tap [trigger](#trigger) while grabbing |
| Snap Mode | hold [trigger](#trigger) while grabbing to engage snap mode, move into position and release to snap |

### Param Interaction
| action | control |
| --- | --- |
| Engage Param | hold [trigger](#trigger) |
| Reset Param to Default | tap [far](#far) while engaging |

### Port/Cable Interaction
| action | control |
| --- | --- |
| Create Cable from Empty Port | hold [trigger](#trigger) |
| Grab Cable End | hold [trigger](#trigger) |
| Destroy Cable | tap [far](#far) while grabbing, or simply drop an un-latched cable |
| Toggle Cable Latched (prevent deletion when dropped) | tap [near](#far) while grabbing |
| Cycle Cable Color | move [analog stick](#stick) left or right (x-axis) while grabbing |
