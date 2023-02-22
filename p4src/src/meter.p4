/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

control MeterBpsTable(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {
    DirectMeter(MeterType_t.BYTES) meter_bps_all;

    action nop() {
        hdr.bg_md.meter_packet_color = (bit<2>)meter_bps_all.execute();
    }    
    
    table meter_bps_table {
        key = {
            hdr.bg_md.meter_bps_idx : exact;
        }

        actions = {
            nop;
        }
        size = METER_BPS_TABLE_SIZE;
        meters = meter_bps_all;
    }

    apply {
        meter_bps_table.apply();
    }
}