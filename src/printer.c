#include "redeye.h"

#include "usb_descriptors.h"
#include "pico/multicore.h"

// Bluetooth support
// ============================================================================
#ifdef CONTROLLER_SUPPORTS_BLUETOOTH
    #include "btstack.h"
    #include "btstack_config.h"
    int btstack_main(int argc, const char * argv[]);
#endif

// Wi-Fi support
// ============================================================================
#ifdef CONTROLLER_SUPPORTS_WIFI
    #include "lwipopts.h"
#endif

// Interface handlers
// ============================================================================
// USB
static inline bool printer_tusb_task()
{
    static uint8_t recv_buffer[TUSB_PACKET_SIZE];
    static uint32_t utf_buffer[TUSB_PACKET_SIZE];
    if(redeye_error || redeye_busy)
        return false;
    if(buffer_full)
    {
        redeye_error = true;
        return false;
    }
    uint32_t bytes_available = tud_printer_read_available();
    if(bytes_available)
    {
        size_t recv_bytes = tud_printer_read(recv_buffer, 
                                            TU_MIN(sizeof(recv_buffer), 
                                            tud_printer_read_available()));
        size_t decoded = utf8_to_utf32(recv_buffer, 
                                       recv_bytes,
                                       utf_buffer);
        for(size_t i = 0; i < decoded; ++i)
            if((utf_buffer[i] != 0x0D) && // Windows CR - restricted, preempt
               (utf_buffer[i] != 0x1B)) // 27 - ESC, restricted, preempt
                buffer_request_write(redeye_transcode_latin1(utf_buffer[i]));
        if(buffer_usage() >= PRINT_BUFFER_HIGHWATER)
            redeye_busy = true;
    }
    return true;
}

// Entry points
// ============================================================================

void core1_thread(void)
{
    board_init();
    static tusb_rhport_init_t dev_init = {
        .role = TUSB_ROLE_DEVICE, 
        .speed = TUSB_SPEED_AUTO
    };
    tusb_init(BOARD_TUD_RHPORT, &dev_init);
    board_init_after_tusb();
    
    while(true)
    {
        tud_task();
        if(!printer_tusb_task())
            continue;
    }
}

int main(void)
{
    multicore_launch_core1(core1_thread);
    redeye_printer_init();
    sleep_ms(3000);
#ifdef SIGNAL_READY
    redeye_putln("Printer ready > ", false);
    redeye_putln("", false);
#endif // SIGNAL_READY
    redeye_set_Latin1();
    buffer_reset();
#ifdef CONTROLLER_SUPPORTS_BLUETOOTH
    btstack_main(0, NULL);
    btstack_run_loop_execute();
#endif
    while(true)
    {
        redeye_print_buffer();
    }
    redeye_printer_shutdown();
    return 0;
}


