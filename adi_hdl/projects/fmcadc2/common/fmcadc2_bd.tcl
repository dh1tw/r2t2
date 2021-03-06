
# ad9625

create_bd_port -dir I rx_ref_clk
create_bd_port -dir O rx_sync
create_bd_port -dir O rx_sysref
create_bd_port -dir I -from 7 -to 0 rx_data_p
create_bd_port -dir I -from 7 -to 0 rx_data_n

# adc peripherals

set axi_ad9625_core [create_bd_cell -type ip -vlnv analog.com:user:axi_ad9625:1.0 axi_ad9625_core]

set axi_ad9625_jesd [create_bd_cell -type ip -vlnv xilinx.com:ip:jesd204:6.0 axi_ad9625_jesd]
set_property -dict [list CONFIG.C_NODE_IS_TRANSMIT {0}] $axi_ad9625_jesd
set_property -dict [list CONFIG.C_LANES {8}] $axi_ad9625_jesd

set axi_ad9625_gt [create_bd_cell -type ip -vlnv analog.com:user:axi_jesd_gt:1.0 axi_ad9625_gt]
set_property -dict [list CONFIG.PCORE_NUM_OF_RX_LANES {8}] $axi_ad9625_gt
set_property -dict [list CONFIG.PCORE_CPLL_FBDIV {1}] $axi_ad9625_gt
set_property -dict [list CONFIG.PCORE_RX_OUT_DIV {1}] $axi_ad9625_gt
set_property -dict [list CONFIG.PCORE_TX_OUT_DIV {1}] $axi_ad9625_gt
set_property -dict [list CONFIG.PCORE_RX_CLK25_DIV {25}] $axi_ad9625_gt
set_property -dict [list CONFIG.PCORE_TX_CLK25_DIV {25}] $axi_ad9625_gt
set_property -dict [list CONFIG.PCORE_PMA_RSV {0x00018480}] $axi_ad9625_gt
set_property -dict [list CONFIG.PCORE_RX_CDR_CFG {0x03000023ff20400020}] $axi_ad9625_gt

set axi_ad9625_dma [create_bd_cell -type ip -vlnv analog.com:user:axi_dmac:1.0 axi_ad9625_dma]
set_property -dict [list CONFIG.C_DMA_TYPE_SRC {1}] $axi_ad9625_dma
set_property -dict [list CONFIG.C_DMA_TYPE_DEST {0}] $axi_ad9625_dma
set_property -dict [list CONFIG.PCORE_ID {0}] $axi_ad9625_dma
set_property -dict [list CONFIG.C_AXI_SLICE_SRC {0}] $axi_ad9625_dma
set_property -dict [list CONFIG.C_AXI_SLICE_DEST {0}] $axi_ad9625_dma
set_property -dict [list CONFIG.C_CLKS_ASYNC_DEST_REQ {1}] $axi_ad9625_dma
set_property -dict [list CONFIG.C_SYNC_TRANSFER_START {0}] $axi_ad9625_dma
set_property -dict [list CONFIG.C_DMA_LENGTH_WIDTH {24}] $axi_ad9625_dma
set_property -dict [list CONFIG.C_2D_TRANSFER {0}] $axi_ad9625_dma
set_property -dict [list CONFIG.C_CYCLIC {0}] $axi_ad9625_dma
set_property -dict [list CONFIG.C_DMA_DATA_WIDTH_SRC {64}] $axi_ad9625_dma
set_property -dict [list CONFIG.C_DMA_DATA_WIDTH_DEST {64}] $axi_ad9625_dma

# connections (gt)

ad_connect  axi_ad9625_gt/ref_clk_c rx_ref_clk
ad_connect  axi_ad9625_gt/rx_data_p rx_data_p
ad_connect  axi_ad9625_gt/rx_data_n rx_data_n
ad_connect  axi_ad9625_gt/rx_sync rx_sync
ad_connect  axi_ad9625_gt/rx_sysref rx_sysref

# connections (adc)

ad_connect  axi_ad9625_gt/tx_clk_g axi_ad9625_gt/tx_clk
ad_connect  axi_ad9625_gt/rx_clk_g axi_ad9625_gt/rx_clk
ad_connect  axi_ad9625_gt/rx_clk_g axi_ad9625_core/rx_clk
ad_connect  axi_ad9625_gt/rx_clk_g axi_ad9625_jesd/rx_core_clk
ad_connect  axi_ad9625_gt/rx_jesd_rst axi_ad9625_jesd/rx_reset
ad_connect  axi_ad9625_gt/rx_sysref axi_ad9625_jesd/rx_sysref

create_bd_cell -type ip -vlnv analog.com:user:util_bsplit:1.0 util_bsplit_rx_gt_charisk
set_property -dict [list CONFIG.CH_DW {4}] [get_bd_cells util_bsplit_rx_gt_charisk]
set_property -dict [list CONFIG.CH_CNT {8}] [get_bd_cells util_bsplit_rx_gt_charisk]

ad_connect  util_bsplit_rx_gt_charisk/data axi_ad9625_gt/rx_gt_charisk
ad_connect  util_bsplit_rx_gt_charisk/split_data_0 axi_ad9625_jesd/gt0_rxcharisk
ad_connect  util_bsplit_rx_gt_charisk/split_data_1 axi_ad9625_jesd/gt1_rxcharisk
ad_connect  util_bsplit_rx_gt_charisk/split_data_2 axi_ad9625_jesd/gt2_rxcharisk
ad_connect  util_bsplit_rx_gt_charisk/split_data_3 axi_ad9625_jesd/gt3_rxcharisk
ad_connect  util_bsplit_rx_gt_charisk/split_data_4 axi_ad9625_jesd/gt4_rxcharisk
ad_connect  util_bsplit_rx_gt_charisk/split_data_5 axi_ad9625_jesd/gt5_rxcharisk
ad_connect  util_bsplit_rx_gt_charisk/split_data_6 axi_ad9625_jesd/gt6_rxcharisk
ad_connect  util_bsplit_rx_gt_charisk/split_data_7 axi_ad9625_jesd/gt7_rxcharisk

create_bd_cell -type ip -vlnv analog.com:user:util_bsplit:1.0 util_bsplit_rx_gt_disperr
set_property -dict [list CONFIG.CH_DW {4}] [get_bd_cells util_bsplit_rx_gt_disperr]
set_property -dict [list CONFIG.CH_CNT {8}] [get_bd_cells util_bsplit_rx_gt_disperr]

ad_connect  util_bsplit_rx_gt_disperr/data axi_ad9625_gt/rx_gt_disperr
ad_connect  util_bsplit_rx_gt_disperr/split_data_0 axi_ad9625_jesd/gt0_rxdisperr
ad_connect  util_bsplit_rx_gt_disperr/split_data_1 axi_ad9625_jesd/gt1_rxdisperr
ad_connect  util_bsplit_rx_gt_disperr/split_data_2 axi_ad9625_jesd/gt2_rxdisperr
ad_connect  util_bsplit_rx_gt_disperr/split_data_3 axi_ad9625_jesd/gt3_rxdisperr
ad_connect  util_bsplit_rx_gt_disperr/split_data_4 axi_ad9625_jesd/gt4_rxdisperr
ad_connect  util_bsplit_rx_gt_disperr/split_data_5 axi_ad9625_jesd/gt5_rxdisperr
ad_connect  util_bsplit_rx_gt_disperr/split_data_6 axi_ad9625_jesd/gt6_rxdisperr
ad_connect  util_bsplit_rx_gt_disperr/split_data_7 axi_ad9625_jesd/gt7_rxdisperr

create_bd_cell -type ip -vlnv analog.com:user:util_bsplit:1.0 util_bsplit_rx_gt_notintable
set_property -dict [list CONFIG.CH_DW {4}] [get_bd_cells util_bsplit_rx_gt_notintable]
set_property -dict [list CONFIG.CH_CNT {8}] [get_bd_cells util_bsplit_rx_gt_notintable]

ad_connect  util_bsplit_rx_gt_notintable/data axi_ad9625_gt/rx_gt_notintable
ad_connect  util_bsplit_rx_gt_notintable/split_data_0 axi_ad9625_jesd/gt0_rxnotintable
ad_connect  util_bsplit_rx_gt_notintable/split_data_1 axi_ad9625_jesd/gt1_rxnotintable
ad_connect  util_bsplit_rx_gt_notintable/split_data_2 axi_ad9625_jesd/gt2_rxnotintable
ad_connect  util_bsplit_rx_gt_notintable/split_data_3 axi_ad9625_jesd/gt3_rxnotintable
ad_connect  util_bsplit_rx_gt_notintable/split_data_4 axi_ad9625_jesd/gt4_rxnotintable
ad_connect  util_bsplit_rx_gt_notintable/split_data_5 axi_ad9625_jesd/gt5_rxnotintable
ad_connect  util_bsplit_rx_gt_notintable/split_data_6 axi_ad9625_jesd/gt6_rxnotintable
ad_connect  util_bsplit_rx_gt_notintable/split_data_7 axi_ad9625_jesd/gt7_rxnotintable

create_bd_cell -type ip -vlnv analog.com:user:util_bsplit:1.0 util_bsplit_rx_gt_data
set_property -dict [list CONFIG.CH_DW {32}] [get_bd_cells util_bsplit_rx_gt_data]
set_property -dict [list CONFIG.CH_CNT {8}] [get_bd_cells util_bsplit_rx_gt_data]

ad_connect  util_bsplit_rx_gt_data/data axi_ad9625_gt/rx_gt_data
ad_connect  util_bsplit_rx_gt_data/split_data_0 axi_ad9625_jesd/gt0_rxdata
ad_connect  util_bsplit_rx_gt_data/split_data_1 axi_ad9625_jesd/gt1_rxdata
ad_connect  util_bsplit_rx_gt_data/split_data_2 axi_ad9625_jesd/gt2_rxdata
ad_connect  util_bsplit_rx_gt_data/split_data_3 axi_ad9625_jesd/gt3_rxdata
ad_connect  util_bsplit_rx_gt_data/split_data_4 axi_ad9625_jesd/gt4_rxdata
ad_connect  util_bsplit_rx_gt_data/split_data_5 axi_ad9625_jesd/gt5_rxdata
ad_connect  util_bsplit_rx_gt_data/split_data_6 axi_ad9625_jesd/gt6_rxdata
ad_connect  util_bsplit_rx_gt_data/split_data_7 axi_ad9625_jesd/gt7_rxdata

ad_connect  axi_ad9625_gt/rx_rst_done axi_ad9625_jesd/rx_reset_done
ad_connect  axi_ad9625_gt/rx_ip_comma_align axi_ad9625_jesd/rxencommaalign_out
ad_connect  axi_ad9625_gt/rx_ip_sync axi_ad9625_jesd/rx_sync
ad_connect  axi_ad9625_gt/rx_ip_sof axi_ad9625_jesd/rx_start_of_frame
ad_connect  axi_ad9625_gt/rx_ip_data axi_ad9625_jesd/rx_tdata
ad_connect  axi_ad9625_gt/rx_data axi_ad9625_core/rx_data
ad_connect  axi_ad9625_fifo/adc_rst axi_ad9625_gt/rx_rst
ad_connect  axi_ad9625_core/adc_clk axi_ad9625_fifo/adc_clk
ad_connect  axi_ad9625_core/adc_enable axi_ad9625_fifo/adc_wr
ad_connect  axi_ad9625_core/adc_data axi_ad9625_fifo/adc_wdata
ad_connect  axi_ad9625_core/adc_dovf axi_ad9625_fifo/adc_wovf
ad_connect  sys_cpu_clk axi_ad9625_fifo/dma_clk
ad_connect  sys_cpu_clk axi_ad9625_dma/s_axis_aclk
ad_connect  sys_cpu_resetn axi_ad9625_dma/m_dest_axi_aresetn
ad_connect  axi_ad9625_fifo/dma_wr axi_ad9625_dma/s_axis_valid
ad_connect  axi_ad9625_fifo/dma_wdata axi_ad9625_dma/s_axis_data
ad_connect  axi_ad9625_fifo/dma_wready axi_ad9625_dma/s_axis_ready
ad_connect  axi_ad9625_fifo/dma_xfer_req axi_ad9625_dma/s_axis_xfer_req

# interconnect (cpu)

ad_cpu_interconnect 0x44A60000 axi_ad9625_gt
ad_cpu_interconnect 0x44A10000 axi_ad9625_core
ad_cpu_interconnect 0x44A91000 axi_ad9625_jesd
ad_cpu_interconnect 0x7c420000 axi_ad9625_dma

# gt uses hp3, and 100MHz clock for both DRP and AXI4

ad_mem_hp3_interconnect sys_cpu_clk sys_ps7/S_AXI_HP3
ad_mem_hp3_interconnect sys_cpu_clk axi_ad9625_gt/m_axi

# interconnect (mem/adc)

ad_mem_hp2_interconnect sys_cpu_clk sys_ps7/S_AXI_HP2
ad_mem_hp2_interconnect sys_cpu_clk axi_ad9625_dma/m_dest_axi

# interrupts

ad_cpu_interrupt ps-13 mb-12 axi_ad9625_dma/irq

