#include "bsp_system.h"

#define UART_PRINTF_BUFFER_SIZE    128U
#define UART0_RX_BUFFER_SIZE       128U
#define UART0_RX_TIMEOUT_MS        10U

static struct rt_ringbuffer uart1_ring_buffer;
static uint8_t uart1_rx_dma_buffer[UART1_DMA_RX_BUFFER_SIZE];
static uint8_t uart1_ring_buffer_input[UART1_BUFFER_SIZE];
static uint16_t uart1_last_dma_read_pos;

static struct rt_ringbuffer uart2_ring_buffer;
static uint8_t uart2_rx_dma_buffer[UART2_DMA_RX_BUFFER_SIZE];
static uint8_t uart2_ring_buffer_input[UART2_BUFFER_SIZE];
static uint16_t uart2_last_dma_read_pos;

static struct rt_ringbuffer uart3_ring_buffer;
static uint8_t uart3_rx_dma_buffer[UART3_DMA_RX_BUFFER_SIZE];
static uint8_t uart3_ring_buffer_input[UART3_BUFFER_SIZE];
static uint16_t uart3_last_dma_read_pos;

static uint8_t uart0_rx_buffer[UART0_RX_BUFFER_SIZE];
static uint8_t uart0_rx_index;
static uint32_t uart0_rx_tick;

static void uart1_dma_rx_poll(void);
static void uart2_dma_rx_poll(void);
static void uart3_dma_rx_poll(void);

void Uart_Tx_Init(void)
{
}

void uart_send_char(UART_Regs *uart, char ch)
{
    if (uart == NULL) {
        return;
    }

    while (DL_UART_Main_isTXFIFOFull(uart) == true) {
        ;
    }

    DL_UART_Main_transmitData(uart, (uint8_t) ch);
}

void uart_send_string(UART_Regs *uart, char *str, int length)
{
    if (uart == NULL || str == NULL || length <= 0) {
        return;
    }

    while (length-- > 0) {
        uart_send_char(uart, *str++);
    }
}

int my_printf(UART_Regs *uart, const char *format, ...)
{
    char buffer[UART_PRINTF_BUFFER_SIZE];
    va_list arg;
    int len;

    if (uart == NULL || format == NULL) {
        return 0;
    }

    va_start(arg, format);
    len = vsnprintf(buffer, sizeof(buffer), format, arg);
    va_end(arg);

    if (len < 0) {
        return len;
    }

    if (len >= (int) sizeof(buffer)) {
        len = (int) sizeof(buffer) - 1;
    }

    uart_send_string(uart, buffer, len);
    return len;
}

void Uart1_Rx_Init(void)
{
    rt_ringbuffer_init(&uart1_ring_buffer, uart1_ring_buffer_input, UART1_BUFFER_SIZE);
    rt_ringbuffer_reset(&uart1_ring_buffer);

    memset(uart1_rx_dma_buffer, 0, sizeof(uart1_rx_dma_buffer));
    uart1_last_dma_read_pos = 0U;

#if defined(UART_1_INST) && defined(DMA_CH0_CHAN_ID)
    DL_DMA_disableChannel(DMA, DMA_CH0_CHAN_ID);
    DL_DMA_setSrcAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t) &UART_1_INST->RXDATA);
    DL_DMA_setDestAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t) uart1_rx_dma_buffer);
    DL_DMA_setTransferSize(DMA, DMA_CH0_CHAN_ID, UART1_DMA_RX_BUFFER_SIZE);
    DL_DMA_setSrcIncrement(DMA, DMA_CH0_CHAN_ID, DL_DMA_ADDR_UNCHANGED);
    DL_DMA_setDestIncrement(DMA, DMA_CH0_CHAN_ID, DL_DMA_ADDR_INCREMENT);
    DL_DMA_enableChannel(DMA, DMA_CH0_CHAN_ID);
#endif

#ifdef UART_1_INST_INT_IRQN
    NVIC_ClearPendingIRQ(UART_1_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_1_INST_INT_IRQN);
#endif
}

void Uart1_Rx_Update(void)
{
    /* Task 侧主动同步 DMA 硬件环，兜底处理连续数据流。 */
    uart1_dma_rx_poll();
}

rt_size_t Uart1_Read(uint8_t *buffer, uint16_t length)
{
    if (buffer == NULL || length == 0U) {
        return 0U;
    }

    return rt_ringbuffer_get(&uart1_ring_buffer, buffer, length);
}

rt_size_t Uart1_Available(void)
{
    return rt_ringbuffer_data_len(&uart1_ring_buffer);
}

void Uart2_Rx_Init(void)
{
    rt_ringbuffer_init(&uart2_ring_buffer, uart2_ring_buffer_input, UART2_BUFFER_SIZE);
    rt_ringbuffer_reset(&uart2_ring_buffer);

    memset(uart2_rx_dma_buffer, 0, sizeof(uart2_rx_dma_buffer));
    uart2_last_dma_read_pos = 0U;

#if defined(UART_2_INST) && defined(DMA_CH1_CHAN_ID)
    DL_DMA_disableChannel(DMA, DMA_CH1_CHAN_ID);
    DL_DMA_setSrcAddr(DMA, DMA_CH1_CHAN_ID, (uint32_t) &UART_2_INST->RXDATA);
    DL_DMA_setDestAddr(DMA, DMA_CH1_CHAN_ID, (uint32_t) uart2_rx_dma_buffer);
    DL_DMA_setTransferSize(DMA, DMA_CH1_CHAN_ID, UART2_DMA_RX_BUFFER_SIZE);
    DL_DMA_setSrcIncrement(DMA, DMA_CH1_CHAN_ID, DL_DMA_ADDR_UNCHANGED);
    DL_DMA_setDestIncrement(DMA, DMA_CH1_CHAN_ID, DL_DMA_ADDR_INCREMENT);
    DL_DMA_enableChannel(DMA, DMA_CH1_CHAN_ID);
#endif

#ifdef UART_2_INST_INT_IRQN
    NVIC_ClearPendingIRQ(UART_2_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_2_INST_INT_IRQN);
#endif
}

void Uart2_Rx_Update(void)
{
    /* Task 侧主动同步 DMA 硬件环，兜底处理连续数据流。 */
    uart2_dma_rx_poll();
}

rt_size_t Uart2_Read(uint8_t *buffer, uint16_t length)
{
    if (buffer == NULL || length == 0U) {
        return 0U;
    }

    return rt_ringbuffer_get(&uart2_ring_buffer, buffer, length);
}

rt_size_t Uart2_Available(void)
{
    return rt_ringbuffer_data_len(&uart2_ring_buffer);
}

void Uart3_Rx_Init(void)
{
    rt_ringbuffer_init(&uart3_ring_buffer, uart3_ring_buffer_input, UART3_BUFFER_SIZE);
    rt_ringbuffer_reset(&uart3_ring_buffer);

    memset(uart3_rx_dma_buffer, 0, sizeof(uart3_rx_dma_buffer));
    uart3_last_dma_read_pos = 0U;

#if defined(UART_3_INST) && defined(DMA_CH2_CHAN_ID)
    DL_DMA_disableChannel(DMA, DMA_CH2_CHAN_ID);
    DL_DMA_setSrcAddr(DMA, DMA_CH2_CHAN_ID, (uint32_t) &UART_3_INST->RXDATA);
    DL_DMA_setDestAddr(DMA, DMA_CH2_CHAN_ID, (uint32_t) uart3_rx_dma_buffer);
    DL_DMA_setTransferSize(DMA, DMA_CH2_CHAN_ID, UART3_DMA_RX_BUFFER_SIZE);
    DL_DMA_setSrcIncrement(DMA, DMA_CH2_CHAN_ID, DL_DMA_ADDR_UNCHANGED);
    DL_DMA_setDestIncrement(DMA, DMA_CH2_CHAN_ID, DL_DMA_ADDR_INCREMENT);
    DL_DMA_enableChannel(DMA, DMA_CH2_CHAN_ID);
#endif

#ifdef UART_3_INST_INT_IRQN
    NVIC_ClearPendingIRQ(UART_3_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_3_INST_INT_IRQN);
#endif
}

void Uart3_Rx_Update(void)
{
    /* Task 侧主动同步 DMA 硬件环，兜底处理连续数据流。 */
    uart3_dma_rx_poll();
}

rt_size_t Uart3_Read(uint8_t *buffer, uint16_t length)
{
    if (buffer == NULL || length == 0U) {
        return 0U;
    }

    return rt_ringbuffer_get(&uart3_ring_buffer, buffer, length);
}

rt_size_t Uart3_Available(void)
{
    return rt_ringbuffer_data_len(&uart3_ring_buffer);
}

static void uart1_dma_rx_poll(void)
{
#if defined(DMA_CH0_CHAN_ID)
    uint16_t remain;
    uint16_t curr_write_pos;

    /* 通过 DMA 剩余传输计数反推出当前写游标位置。 */
    remain = DL_DMA_getTransferSize(DMA, DMA_CH0_CHAN_ID);
    if (remain > UART1_DMA_RX_BUFFER_SIZE) {
        return;
    }

    curr_write_pos = (uint16_t) (UART1_DMA_RX_BUFFER_SIZE - remain);
    if (curr_write_pos == uart1_last_dma_read_pos) {
        return;
    }

    if (curr_write_pos > uart1_last_dma_read_pos) {
        uint16_t len = (uint16_t) (curr_write_pos - uart1_last_dma_read_pos);
        rt_ringbuffer_put(&uart1_ring_buffer, &uart1_rx_dma_buffer[uart1_last_dma_read_pos], len);
    }
    else {
        uint16_t len1 = (uint16_t) (UART1_DMA_RX_BUFFER_SIZE - uart1_last_dma_read_pos);
        rt_ringbuffer_put(&uart1_ring_buffer, &uart1_rx_dma_buffer[uart1_last_dma_read_pos], len1);

        if (curr_write_pos > 0U) {
            rt_ringbuffer_put(&uart1_ring_buffer, &uart1_rx_dma_buffer[0], curr_write_pos);
        }
    }

    uart1_last_dma_read_pos = curr_write_pos;
#endif
}

static void uart2_dma_rx_poll(void)
{
#if defined(DMA_CH1_CHAN_ID)
    uint16_t remain;
    uint16_t curr_write_pos;

    /* 通过 DMA 剩余传输计数反推出当前写游标位置。 */
    remain = DL_DMA_getTransferSize(DMA, DMA_CH1_CHAN_ID);
    if (remain > UART2_DMA_RX_BUFFER_SIZE) {
        return;
    }

    curr_write_pos = (uint16_t) (UART2_DMA_RX_BUFFER_SIZE - remain);
    if (curr_write_pos == uart2_last_dma_read_pos) {
        return;
    }

    if (curr_write_pos > uart2_last_dma_read_pos) {
        uint16_t len = (uint16_t) (curr_write_pos - uart2_last_dma_read_pos);
        rt_ringbuffer_put(&uart2_ring_buffer, &uart2_rx_dma_buffer[uart2_last_dma_read_pos], len);
    }
    else {
        uint16_t len1 = (uint16_t) (UART2_DMA_RX_BUFFER_SIZE - uart2_last_dma_read_pos);
        rt_ringbuffer_put(&uart2_ring_buffer, &uart2_rx_dma_buffer[uart2_last_dma_read_pos], len1);

        if (curr_write_pos > 0U) {
            rt_ringbuffer_put(&uart2_ring_buffer, &uart2_rx_dma_buffer[0], curr_write_pos);
        }
    }

    uart2_last_dma_read_pos = curr_write_pos;
#endif
}

static void uart3_dma_rx_poll(void)
{
#if defined(DMA_CH2_CHAN_ID)
    uint16_t remain;
    uint16_t curr_write_pos;

    /* 通过 DMA 剩余传输计数反推出当前写游标位置。 */
    remain = DL_DMA_getTransferSize(DMA, DMA_CH2_CHAN_ID);
    if (remain > UART3_DMA_RX_BUFFER_SIZE) {
        return;
    }

    curr_write_pos = (uint16_t) (UART3_DMA_RX_BUFFER_SIZE - remain);
    if (curr_write_pos == uart3_last_dma_read_pos) {
        return;
    }

    if (curr_write_pos > uart3_last_dma_read_pos) {
        uint16_t len = (uint16_t) (curr_write_pos - uart3_last_dma_read_pos);
        rt_ringbuffer_put(&uart3_ring_buffer, &uart3_rx_dma_buffer[uart3_last_dma_read_pos], len);
    }
    else {
        uint16_t len1 = (uint16_t) (UART3_DMA_RX_BUFFER_SIZE - uart3_last_dma_read_pos);
        rt_ringbuffer_put(&uart3_ring_buffer, &uart3_rx_dma_buffer[uart3_last_dma_read_pos], len1);

        if (curr_write_pos > 0U) {
            rt_ringbuffer_put(&uart3_ring_buffer, &uart3_rx_dma_buffer[0], curr_write_pos);
        }
    }

    uart3_last_dma_read_pos = curr_write_pos;
#endif
}

#ifdef UART_0_INST
void UART_0_INST_IRQHandler(void)
{
    switch (DL_UART_getPendingInterrupt(UART_0_INST)) {
    case DL_UART_IIDX_RX:
        uart0_rx_buffer[uart0_rx_index++] = DL_UART_Main_receiveData(UART_0_INST);
        uart0_rx_tick = uwTick;

        if (uart0_rx_index >= UART0_RX_BUFFER_SIZE) {
            uart0_rx_index = 0U;
        }
        break;
    default:
        break;
    }
}
#endif

#ifdef UART_1_INST
void UART_1_INST_IRQHandler(void)
{
    switch (DL_UART_Main_getPendingInterrupt(UART_1_INST)) {
    case DL_UART_MAIN_IIDX_RX_TIMEOUT_ERROR:
        /* IRQ 侧 poll: 一帧停下来时尽快收口，降低延迟。 */
        uart1_dma_rx_poll();
        break;
    default:
        break;
    }
}
#endif

#ifdef UART_2_INST
void UART_2_INST_IRQHandler(void)
{
    switch (DL_UART_Main_getPendingInterrupt(UART_2_INST)) {
    case DL_UART_MAIN_IIDX_RX_TIMEOUT_ERROR:
        /* IRQ 侧 poll: 一帧停下来时尽快收口，降低延迟。 */
        uart2_dma_rx_poll();
        break;
    default:
        break;
    }
}
#endif

#ifdef UART_3_INST
void UART_3_INST_IRQHandler(void)
{
    switch (DL_UART_Main_getPendingInterrupt(UART_3_INST)) {
    case DL_UART_MAIN_IIDX_RX_TIMEOUT_ERROR:
        /* IRQ 侧 poll: 一帧停下来时尽快收口，降低延迟。 */
        uart3_dma_rx_poll();
        break;
    default:
        break;
    }
}
#endif

void uart0_task(void)
{
    if (uart0_rx_index == 0U) {
        return;
    }

    if ((uwTick - uart0_rx_tick) >= UART0_RX_TIMEOUT_MS) {
        my_printf(UART_0_INST, "%s\r\n", uart0_rx_buffer);
        memset(uart0_rx_buffer, 0, sizeof(uart0_rx_buffer));
        uart0_rx_index = 0U;
    }
}
