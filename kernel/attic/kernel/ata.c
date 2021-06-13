/*************************************************************************/
/*
 * $Id: ata.c 44 2010-02-19 23:53:25Z kfiresun $ 
 */
/*************************************************************************/

#include "cdefs.h"
#include "cpu.h"
#include "string.h"
#include "stdlib.h"
#include "kernio.h"
#include "ktime.h"
#include "kheap.h"
#include "ata.h"

/*************************************************************************/

#define ATAREG_DATA         /* base_port1 */ 0
#define ATAREG_ERROR        /* base_port1 */ 1
#define ATAREG_FEATURES     /* base_port1 */ 1
#define ATAREG_SECTOR_CNT   /* base_port1 */ 2
#define ATAREG_LBA_LOW      /* base_port1 */ 3
#define ATAREG_LBA_MID      /* base_port1 */ 4
#define ATAREG_LBA_HIGH     /* base_port1 */ 5
#define ATAREG_HEAD         /* base_port1 */ 6
#define ATAREG_STATUS       /* base_port1 */ 7
#define ATAREG_COMMAND      /* base_port1 */ 7
#define ATAREG_ALT_STATUS   /* base_port2 */ 0
#define ATAREG_DEV_CONTROL  /* base_port2 */ 0

/*************************************************************************/

ata_bus_t *s_buses; /* Attached ATA buses */
size_t s_num_buses; /* Number of buses */

/*************************************************************************/
/*
 * Reads an ATA buses status register.
 *
 * Parameters:
 *    bus	- The bus to read from.
 *    delay	- If true, the first four reads are ignored.
 *
 * Note:
 *   Sometimes it is necessary to delay reading the status register until
 * a device is fully selected.  Use the delay parameter to do this.
 *
 * Unlike wait_for_status() below, this function only performs one read
 * and immediately returns with that value.
 */
uint8_t read_status(ata_bus_t *bus, bool_t delay)
{
    if (delay)
    {
        int i;

        for (i = 0; i < 4; ++i)
            bus_read_1(bus->base_port1, ATAREG_STATUS);
    }

    return bus_read_1(bus->base_port1, ATAREG_STATUS);
}

/*************************************************************************/
/*
 * Wait for a drive to be !BSY, and mask in its status register.
 *
 * Returns false on timeout, true on success.
 *
 * Parameters:
 *    bus	- The ATA bus we're waiting to become ready on.
 *    mask	- The bit(s) to check.
 *    bits	- The bit patern to match to.
 *    timeout	- The timeout value in miliseconds.
 */
static bool_t ata_wait(ata_bus_t *bus, int mask, int bits, uint32_t timeout)
{
    ktimer_t timer;
    uint8_t status;

    if (timeout == 0)
        return false;

    init_ktimer(&timer, timeout);

    for (;;)
    {
        status = bus_read_1(bus->base_port1, ATAREG_STATUS);

        if ((status & (ATA_STATUS_BSY | mask)) == bits)
            break;

        if (!check_ktimer(&timer))
            return false;
    }

    return true;
}

/*************************************************************************/
/*
 * Wait the ATA device tell us its ready to be read from, and read data
 * from it.
 */
static int wait_read(ata_bus_t *bus, ata_command_t *cmd)
{
    ktimer_t timer;

    if (cmd->timeout > 0)
        init_ktimer(&timer, cmd->timeout);

    for (;;)
    {
        if (cmd->status & ATA_STATUS_ERR)
        {
            cmd->error = bus_read_1(bus->base_port1, ATAREG_ERROR);

            if (cmd->error != 0)
                return ATA_ERR_ABRTCMD;
        }

        if (cmd->status & ATA_STATUS_DRQ)
        {
            bus_read_s2(bus->base_port1, ATAREG_DATA, cmd->data, cmd->data_sz);
            break;
        }

        kwait(100);
        cmd->status = bus_read_1(bus->base_port1, ATAREG_STATUS);

        if (cmd->timeout > 0)
        {
            if (!check_ktimer(&timer))
                return ATA_ERR_TIMEOUT;
        }
    }

    return ATA_ERR_NOERR;
}

/*************************************************************************/
/*
 * Wait for the ATA device to tell us that its ready to be written to, and
 * write data to it.
 */
static int wait_write(ata_bus_t *bus, ata_command_t *cmd)
{
    ktimer_t timer;

    if (cmd->timeout > 0)
	init_ktimer(&timer, cmd->timeout);

    for (;;)
    {
        if (cmd->status & ATA_STATUS_ERR)
        {
            cmd->error = bus_read_1(bus->base_port1, ATAREG_ERROR);

            if (cmd->error != 0)
                return ATA_ERR_ABRTCMD;
        }

        if (cmd->status & ATA_STATUS_DRQ)
        {
            bus_write_s2(bus->base_port1, ATAREG_DATA, cmd->data, cmd->data_sz);
            break;
        }

        kwait(100);
        cmd->status = bus_read_1(bus->base_port2, ATAREG_ALT_STATUS);

        if (cmd->timeout > 0)
        {
            if (!check_ktimer(&timer))
                return ATA_ERR_TIMEOUT;
        }
    }

    return ATA_ERR_NOERR;
}

/*************************************************************************/
/*
 * Must be called with interrupts disabled!
 *
 * Lower level ATA bus command execution.
 *
 * By this point the device should not be busy.
 */
static int exec_command(ata_bus_t *bus, ata_command_t *cmd)
{
    int rval = ATA_ERR_NOERR;

    // Drive should already be selected.
    //bus_write_1(bus->base_port1, ATAREG_HEAD, cmd->head);

    /*
    bus_write_1(bus->base_port1, ATAREG_FEATURES, cmd->features);

    bus_write_1(bus->base_port1, ATAREG_SECTOR_CNT, cmd->sect_cnt);

    bus_write_1(bus->base_port1, ATAREG_LBA_LOW, cmd->cylinder & 0xSOMEMASK);
    bus_write_1(bus->base_port1, ATAREG_LBA_MID, cmd->cylinder & 0xSOMEOTHERMASK);
    bus_write_1(bus->base_port1, ATAREG_LBA_HIGH, cmd->cylinder & 0xYETANOTHERMASK);
    */

    bus_write_1(bus->base_port1, ATAREG_COMMAND, cmd->command);

    cmd->status = read_status(bus, true);
    kwait(400);
    cmd->status = read_status(bus, true);
    cmd->error = bus_read_1(bus->base_port1, ATAREG_ERROR);

    if (cmd->flags & ATA_FLAG_DATA)
    {
        if (cmd->flags & ATA_FLAG_READ)
            rval = wait_read(bus, cmd);
        else
            rval = wait_write(bus, cmd);
    }

    return rval;
}

/*************************************************************************/
/*
 * Selects an ATA device, and waits for it to become ready.
 *
 * Returns false if the device did not become ready within the timeout
 * period.  True if the device is ready.
 *
 * PARAMETERS:
 *    bus	- The ATA bus the device is on.
 *    head	- Value to send to the ATA head register.
 *    timeout	- Timeout period for the device to become ready in miliseconds.
 */
bool_t select_device(ata_bus_t *bus, int head, int timeout)
{
    bus_write_1(bus->base_port1, ATAREG_HEAD, head);
    kwait(400);

    return ata_wait(bus, ATA_STATUS_RDY, ATA_STATUS_RDY, timeout);
}

/*************************************************************************/
/*
 * Executes an ATA bus command.
 */
int do_ata_command(ata_bus_t *bus, ata_command_t *cmd)
{
    int rval = ATA_ERR_NOERR;

    cmd->status = 0;
    cmd->error = 0;

    /* Sanity check... */
    if (((cmd->flags & ATA_FLAG_DATA) != 0) && (!cmd->data || !cmd->data_sz))
        return ATA_ERR_BADPARAM;

    /* Select drive and wait for it to finish with whatever it's doing. */
    if (!select_device(bus, cmd->head, cmd->timeout))
        return ATA_ERR_TIMEOUT;

    block_interrupts();

    /* Issue our command */
    rval = exec_command(bus, cmd);

    start_interrupts();

    return rval;
}

/*************************************************************************/
/*
 * Resets the ATA bus.
 *
 * Returns true if we successfully reset the bus.  False if not usually 
 * a timeout.
 */
bool_t reset_bus(ata_bus_t *bus)
{
    uint8_t status;

    /* 
     * The ATA docs have this first byte as 0x08, but the example code
     * I found has this as 0x04....
     */
    bus_write_1(bus->base_port1, ATAREG_COMMAND, 0x04);
    bus_write_1(bus->base_port1, ATAREG_COMMAND, 0x00);

    status = bus_read_1(bus->base_port1, ATAREG_STATUS);

    if (status & (ATA_STATUS_BSY | ATA_STATUS_DRQ))
    {
        if (!ata_wait(bus, ATA_STATUS_RDY, ATA_STATUS_RDY, 1000))
        {
            kprintf("ATA bus %d timed out waiting for reset.\n", bus->id);
            return false;
        }
    }

    return true;
}

/*************************************************************************/
/*
 * Fixes an ATAP string to something more meaningful to the rest 
 * of the system.
 */
static void fix_atap_str(char *str, size_t sz)
{
    strbswp(str, sz);
    str[sz - 1] = '\0';

    strtrim(str, sz);
}

/*************************************************************************/

int get_ata_params(ata_bus_t *bus, int drive, ata_params_t *prms)
{
    uint8_t lba_lo, lba_hi;
    ata_command_t cmd;
    int err;

    memset(&cmd, 0, sizeof(ata_command_t));
    memset(prms, 0, sizeof(ata_params_t));

    cmd.head = (drive == 1) ? 0x10 : 0;

    /* Check for magic bytes to see if this is an ATA or ATAPI device. */
    if (!select_device(bus, cmd.head, 1000))
        return ATA_ERR_TIMEOUT;

    bus_read_1(bus->base_port1, ATAREG_SECTOR_CNT);
    bus_read_1(bus->base_port1, ATAREG_LBA_LOW);
    lba_lo = bus_read_1(bus->base_port1, ATAREG_LBA_MID);
    lba_hi = bus_read_1(bus->base_port1, ATAREG_LBA_HIGH);

    if ((lba_lo == 0x14) && (lba_hi == 0xEB))
        cmd.command = ATAPI_IDENTIFY_DEVICE;
    else
        cmd.command = WDCC_IDENTIFY;

    cmd.flags = ATA_FLAG_READ;
    cmd.timeout = 1000;
    cmd.data = prms;
    cmd.data_sz = sizeof(ata_params_t);

    err = do_ata_command(bus, &cmd);

    if ((err == ATA_ERR_NOERR) && ((cmd.status & ATA_STATUS_ERR) == 0))
    {
        fix_atap_str(prms->atap_model, ATA_SZ_MODEL);
        fix_atap_str(prms->atap_revision, ATA_SZ_REVISION);
        fix_atap_str(prms->atap_serial, ATA_SZ_SERIAL);
    }
    else
    {
        kprintf("ATA bus %d DEVICE %d returned error on ID: %d 0x%02X\n", bus->id, drive, err, cmd.error);
    }

    return err;   
}

/*************************************************************************/

void init_bus(ata_bus_t *bus)
{
    /* 
     * Read the status register, if we get a value of 0xFF, then there are
     * no devices on this bus.
     */
    uint8_t status = bus_read_1(bus->base_port1, ATAREG_STATUS);

    bus->dev_count = 0;

    if (status != 0xFF)
    {
        ata_params_t prms;
        ata_device_t *dev;
        int err, i;

        for (i = 0; i < 2; ++i)
        {
            err = get_ata_params(bus, i, &prms);

            if (err == ATA_ERR_NOERR)
            {
                dev = (ata_device_t *)kmalloc(sizeof(ata_device_t));
                memset(dev, 0, sizeof(ata_device_t));

                dev->id = (i == 1) ? 0x10 : 0;

                strkcpy(dev->model, ATA_SZ_MODEL, prms.atap_model, ATA_SZ_MODEL);

                strkcpy(dev->revision, ATA_SZ_REVISION, prms.atap_revision, ATA_SZ_REVISION);

                strkcpy(dev->serial, ATA_SZ_SERIAL, prms.atap_serial, ATA_SZ_SERIAL);

                if (prms.atap_sec_st & WDC_SEC_ESE_SUPP)
                    dev->sec_flags |= ATADEV_SEC_SUPPORTED;

                if (prms.atap_sec_st & WDC_SEC_FROZEN)
                    dev->sec_flags |= ATADEV_SEC_FROZEN;

                if (prms.atap_sec_st & WDC_SEC_LOCKED)
                {
                    dev->sec_flags |= ATADEV_SEC_LOCKED | ATADEV_SEC_SUPPORTED;
                }

                if (prms.atap_sec_st & WDC_SEC_EN)
                {
                    dev->sec_flags |= ATADEV_SEC_ENABLED | ATADEV_SEC_SUPPORTED;
                }

                if (prms.atap_sec_st & WDC_SEC_EXP)
                    dev->sec_flags |= ATADEV_SEC_COUNT_EXP;

                if (prms.atap_sec_st & WDC_SEC_ESE_SUPP)
                    dev->sec_flags |= ATADEV_SEC_SUPP_ENHANCED;

                /* Add to list now */
                bus->devices[i] = dev;
                bus->dev_count++;
            }
            else
                bus->devices[i] = NULL;
        }
    }
    else
        kprintf("ATA bus %d is floating, assuming no devices.\n", bus->id);
}

/*************************************************************************/
/*
 * Initializes the ATA buses.
 */
void init_ata(void)
{
    s_num_buses = 2;
    s_buses = (ata_bus_t *)kmalloc(sizeof(ata_bus_t) * s_num_buses);
    memset(s_buses, 0, sizeof(ata_bus_t) * s_num_buses);

    s_buses[0].id = 1;
    s_buses[0].base_port1 = (io_port_t)0x01F0;
    s_buses[0].base_port2 = (io_port_t)0x03F6;
    init_bus(&s_buses[0]);

    s_buses[1].id = 2;
    s_buses[1].base_port1 = (io_port_t)0x0170;
    s_buses[1].base_port2 = (io_port_t)0x0376;
    init_bus(&s_buses[1]);

    kprintf("ATA initilaized\n");
}

/*************************************************************************/
/*
 * Returns an ATA bus based on its ID.
 */
ata_bus_t *get_ata_bus(unsigned int id)
{
    if ((id == 0) || (id > s_num_buses))
        return NULL;

    return &s_buses[id - 1];
}

/*************************************************************************/

void fill_security_cmd(ata_device_t *dev, ata_command_t *cmd, atasecbuffer_t *params, const char *passwd, size_t psz)
{
    memset(cmd, 0, sizeof(ata_command_t));

    cmd->head = dev->id;  
    cmd->timeout = 12000;

    if (params != NULL)
    {
        cmd->flags = ATA_FLAG_WRITE;

        cmd->data = params;
        cmd->data_sz = sizeof(atasecbuffer_t);

        params->data.flags = SEC_FLAG_USER | SEC_FLAG_NORMAL_ERASE | SEC_FLAG_LEVEL_HIGH;

        if (passwd != NULL)
        {
            strkcpy((char *)params->data.password, ATA_SEC_PASSWORD_SIZE,passwd, psz);
        }
    }   
}

/*************************************************************************/

int set_device_password(ata_bus_t *bus, ata_device_t *dev, const char *passwd, size_t psz)
{
    int drive = dev->id ? 1 : 0;
    atasecbuffer_t params;
    ata_command_t cmd;
    
    fill_security_cmd(dev, &cmd, &params, passwd, psz);    
    cmd.command = WDCC_SECURITY_SET_PASS;

    kprintf("ATA bus %d DEVICE %d: Setting password\n", bus->id, drive);

    return do_ata_command(bus, &cmd);
}

/*************************************************************************/

int erase_device(ata_bus_t *bus, ata_device_t *dev, const char *passwd, size_t psz)
{
    int drive = dev->id ? 1 : 0;
    atasecbuffer_t params;
    ata_command_t cmd;
    int err;

    fill_security_cmd(dev, &cmd, NULL, NULL, 0);
    cmd.command = WDCC_SECURITY_ERASE_PREPAIR;

    kprintf("ATA bus %d DEVICE %d: Prepairing for erase\n",
	bus->id, drive);
    err = do_ata_command(bus, &cmd);

    if (err == ATA_ERR_NOERR)
    {
        fill_security_cmd(dev, &cmd, &params, passwd, psz);
        cmd.command = WDCC_SECURITY_ERASE_UNIT;

        kprintf("ATA bus %d DEVICE %d: Sending erase command\n", bus->id, drive);

        err = do_ata_command(bus, &cmd);

        if (err != ATA_ERR_NOERR)
        {
            kprintf("ATA bus %d DEVICE %d: Error sending erase command: %d\n",
            bus->id, drive, err);
        }
    }
    else
    {
        kprintf("ATA bus %d DEVICE %d: Unable to prepair for erase: %d\n", bus->id, dev->id, err);
    }

    return err;
}

/*************************************************************************/

void wipe_drive(ata_bus_t *bus, int drive, const char *passwd, size_t psz)
{
    ata_device_t *dev = bus->devices[drive];
    int err;

    if (dev == NULL)
    {
        kprintf("wipe_drive(): ATA bus %d DEVICE %d not attached.\n", bus->id, drive);
        return;
    }

    if ((dev->sec_flags & ATADEV_SEC_SUPPORTED) == 0)
    {
        kprintf("wipe_drive(): ATA bus %d DEVICE %d does not support secure erase.\n", bus->id, drive);
        return;
    }

    if (dev->sec_flags & ATADEV_SEC_FROZEN)
    {
        kprintf("wipe_drive(): ATA bus %d DEVICE %d is frozen.\n", bus->id, drive);
        return;
    }

    if ((dev->sec_flags & ATADEV_SEC_LOCKED) == 0)
    {
        err = set_device_password(bus, dev, passwd, psz);

        if (err != ATA_ERR_NOERR)
        {
            kprintf("wipe_drive(): ATA bus %d DEVICE %d returned error: %d\n", bus->id, drive, err);
            return;
        }
    }

    err = erase_device(bus, dev, passwd, psz);

    if (err != ATA_ERR_NOERR)
    {
        kprintf("wipe_drive(): ATA bus %d DEVICE %d returned error: %d\n", bus->id, drive, err);
        return;
    }

    kprintf("wipe_drive(): ATA bus %d DEVICE %d drive is wiping....\n", bus->id, drive);
}

/*************************************************************************/
