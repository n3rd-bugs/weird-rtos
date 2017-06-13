/*
 * net_route.c
 *
 * Copyright (c) 2017 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any outcome from it's direct
 * or indirect use.
 */
#include <os.h>

#ifdef CONFIG_NET
#include <net.h>
#include <net_route.h>

/* Routes database. */
static NET_ROUTE routes[NET_NUM_ROUTES];

/* Internal function prototypes. */
static int32_t route_lock();
static void route_unlock();

/*
 * route_lock
 * This function will get lock for route database.
 */
static int32_t route_lock()
{
    SYS_LOG_FUNTION_ENTRY(ROUTE);

    /* Lock scheduler. */
    scheduler_lock();

    SYS_LOG_FUNTION_EXIT(ROUTE);

    /* Return success. */
    return (SUCCESS);

} /* route_lock */

/*
 * route_unlock
 * This function will release lock for route database.
 */
static void route_unlock()
{
    SYS_LOG_FUNTION_ENTRY(ROUTE);

    /* Enable scheduling. */
    scheduler_unlock();

    SYS_LOG_FUNTION_EXIT(ROUTE);

} /* route_unlock */

/*
 * route_add
 * @device: Network interface to use for this device.
 * @iface_addr: Interface address to be used for this route.
 * @gateway: Gateway address for this route.
 * @destination: Destination address for this route.
 * @subnet: Subnet mask for this route.
 * @flags: Initial flags for this route.
 * @return: Success will be returned if a route was added,
 *  ROUTE_EXIST will be returned if an existing route was found,
 *  ROUTE_NO_MEM will be returned if we were unable to find a free slot
 *  for this route.
 * This function will add a new route.
 */
int32_t route_add(FD device, uint32_t interface_address, uint32_t gateway, uint32_t destination, uint32_t subnet, uint8_t flags)
{
    int32_t i, free_route = -1, status;

    SYS_LOG_FUNTION_ENTRY(ROUTE);

    /* Lock the routes. */
    status = route_lock();

    if (status == SUCCESS)
    {
        /* Traverse the route list. */
        for (i = 0; i < NET_NUM_ROUTES; i++)
        {
            /* If we already have such route. */
            if ((routes[i].flags & ROUTE_VALID) && (interface_address == routes[i].interface_address) && (gateway == routes[i].gateway) && (destination == routes[i].destination) && (subnet == routes[i].subnet))
            {
                /* Route already exist. */
                status = ROUTE_EXIST;
                break;
            }
            else if (((routes[i].flags & ROUTE_VALID) == 0) && (free_route == -1))
            {
                /* Save the free route index. */
                free_route = i;
            }
        }

        if (status == SUCCESS)
        {
            /* If we have a free  route entry. */
            if (free_route >= 0)
            {
                /* Update the route entry. */
                routes[free_route].device = device;
                routes[free_route].gateway = gateway;
                routes[free_route].destination = destination;
                routes[free_route].subnet = subnet;
                routes[free_route].interface_address = interface_address;

                SYS_LOG_FUNTION_MSG(ROUTE, SYS_LOG_INFO, "adding %d.%d.%d.%d/%d.%d.%d.%d through %d.%d.%d.%d at %s", SYS_LOG_IP(destination), SYS_LOG_IP(subnet), SYS_LOG_IP(gateway), ((FS *)device)->name);

                /* Mark this route as valid. */
                routes[free_route].flags = (flags | ROUTE_VALID);
            }
            else
            {
                /* We don't have memory for a new route. */
                status = ROUTE_NO_MEM;
            }
        }

        /* Unlock the routes. */
        route_unlock();
    }

    SYS_LOG_FUNTION_EXIT_STATUS(ROUTE, status);

    /* Return status to the caller. */
    return (status);

} /* route_add */

/*
 * route_remove
 * @device: Network interface for which routes are to be removed.
 * @gateway: Gateway address for which routes are to be removed.
 * @destination: Destination address for which routes are to be removed.
 * @return: Success will be returned if required routes were removed.
 * This function will remove routes for a given criteria.
 */
int32_t route_remove(FD device, uint32_t gateway, uint32_t destination)
{
    int32_t i, status;

    SYS_LOG_FUNTION_ENTRY(ROUTE);

    /* Lock the routes. */
    status = route_lock();

    if (status == SUCCESS)
    {
        /* Traverse the route list. */
        for (i = 0; i < NET_NUM_ROUTES; i++)
        {
            /* If this is a valid route. */
            if (routes[i].flags & ROUTE_VALID)
            {
                /* If we need to remove route for a device or a specific
                 * destination. */
                if (((device == 0x0) || (routes[i].device == device)) &&
                    ((gateway == 0x0) || (routes[i].gateway == gateway)) &&
                    ((destination == 0x0) || (routes[i].destination == destination)))
                {
                    /* Mark this route as invalid. */
                    routes[i].flags = 0;
                }
            }
        }

        /* Unlock the routes. */
        route_unlock();
    }

    SYS_LOG_FUNTION_EXIT_STATUS(ROUTE, status);

    /* Return status to the caller. */
    return (status);

} /* route_remove */

/*
 * route_get
 * @device: Network interface associated for found route is returned here, if
 *  this is already set then a route will be searched for that device only.
 * @destination: Destination address for which a route is required.
 * @iface_addr: Interface address to use to communicate.
 * @gw_addr: Gateway address to use to communicate.
 * @subnet: Subnet mask for this route will be returned here.
 * @return: Success will be returned if a valid route was found,
 *  NET_DST_UNREACHABLE will be returned if a route was not found.
 * This function will get a destination route for the given address.
 */
int32_t route_get(FD *device, uint32_t destination, uint32_t *iface_addr, uint32_t *gw_addr, uint32_t *subnet)
{
    int32_t i, status, route = -1, default_route = -1;
    uint32_t this_match, match = 0;

    SYS_LOG_FUNTION_ENTRY(ROUTE);

    /* Lock the routes. */
    status = route_lock();

    if (status == SUCCESS)
    {
        /* Traverse the route list. */
        for (i = 0; i < NET_NUM_ROUTES; i++)
        {
            /* If we already have such route. */
            if (routes[i].flags & ROUTE_VALID)
            {
                /* If we are trying to find route for a broadcast address,
                 * only use a link local address for broadcast. */
                if (((device == NULL) || (*device == NULL) || (routes[i].device == *device)) && ((iface_addr == NULL) || (*iface_addr == IPV4_ADDR_UNSPEC) || (routes[i].interface_address == *iface_addr)) && (destination == IPV4_ADDR_BCAST) && (routes[i].gateway == IPV4_GATWAY_LL))
                {
                    /* Return network for this address. */
                    route = i;
                    match = IPV4_SUBNET_LL;
                }

                /* If this is a default gateway and has the same device. */
                else if (((device == NULL) || (*device == NULL) || (routes[i].device == *device)) && ((iface_addr == NULL) || (*iface_addr == IPV4_ADDR_UNSPEC)) && (destination != IPV4_ADDR_BCAST) && (routes[i].subnet == 0x0))
                {
                    /* Save the default route. */
                    default_route = i;
                }

                /* If we can use this route and has the same device. */
                else if (((device == NULL) || (*device == NULL) || (routes[i].device == *device)) && ((iface_addr == NULL) || (*iface_addr == IPV4_ADDR_UNSPEC)) && (destination & routes[i].subnet) == (routes[i].destination & routes[i].subnet))
                {
                    /* Match the route. */
                    this_match = destination & routes[i].subnet;

                    /* If this is route is more accurate. */
                    if ((this_match != 0) && ((this_match > match) || ((this_match == match) && (routes[i].subnet > routes[route].subnet))))
                    {
                        /* This is more accurate route. */
                        route =  i;
                        match = this_match;
                    }
                }
            }
        }

        /* If we have a route. */
        if (route >= 0)
        {
            /* If we need to return the device associated for this route. */
            if ((device != NULL) && (*device == NULL))
            {
                /* Return the device we need to use. */
                *device = routes[route].device;
            }

            /* If we need to return the interface address for this route. */
            if ((iface_addr != NULL) && (*iface_addr == IPV4_ADDR_UNSPEC))
            {
                /* Return the source address we need to use. */
                *iface_addr = routes[route].interface_address;
            }

            /* If we need to return the gateway address for this route. */
            if (gw_addr != NULL)
            {
                /* If this is a link local route. */
                if (routes[route].gateway == IPV4_GATWAY_LL)
                {
                    /* For on link route use the node as gateway. */
                    *gw_addr = destination;
                }
                else
                {
                    /* Populate the destination address. */
                    *gw_addr = routes[route].gateway;
                }
            }

            /* If need to return the subnet for this route. */
            if (subnet != NULL)
            {
                /* Return associated subnet mask. */
                *subnet = routes[route].subnet;
            }
        }

        /* If we found a default route. */
        else if (default_route >= 0)
        {
            /* If we need to return the device associated for this route. */
            if ((device != NULL) && (*device == NULL))
            {
                /* Return associated device. */
                *device = routes[default_route].device;
            }

            /* If we need to return the interface address for this route. */
            if ((iface_addr != NULL) && (*iface_addr == IPV4_ADDR_UNSPEC))
            {
                /* Return the source address we need to use. */
                *iface_addr = routes[default_route].interface_address;
            }

            /* If we need to return the gateway address for this route. */
            if (gw_addr != NULL)
            {
                /* Return gateway address. */
                *gw_addr = routes[default_route].gateway;
            }

            /* If need to return the subnet for this route. */
            if (subnet != NULL)
            {
                /* Return associated subnet mask. */
                *subnet = routes[default_route].subnet;
            }
        }

        else
        {
            /* A route was not found for the destination. */
            status = NET_DST_UNREACHABLE;
        }

        /* Unlock the routes. */
        route_unlock();
    }

    SYS_LOG_FUNTION_EXIT_STATUS(ROUTE, status);

    /* Return status to the caller. */
    return (status);

} /* route_get */

/*
 * route_print
 * This function will print all the routes in the system.
 */
void route_print()
{
    int32_t i;
    FS *net_fs;

    SYS_LOG_FUNTION_ENTRY(ROUTE);

    /* Lock the routes. */
    if (route_lock() == SUCCESS)
    {
        /* Traverse the route list. */
        for (i = 0; i < NET_NUM_ROUTES; i++)
        {
            /* If we already have such route. */
            if (routes[i].flags & ROUTE_VALID)
            {
                net_fs = (FS*)routes[i].device;

                /* Print this routes information. */
                printf("%s ", net_fs->name);
                printf("@ 0x%08lX", routes[i].gateway);
                printf(" 0x%08lX/", routes[i].destination);
                printf("0x%08lX\r\n", routes[i].subnet);
            }
        }

        /* Unlock the routes. */
        route_unlock();
    }

    SYS_LOG_FUNTION_EXIT(ROUTE);

} /* route_print */

#endif /* CONFIG_NET */
