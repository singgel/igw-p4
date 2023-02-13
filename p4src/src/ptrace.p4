/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

control EgressPtrace(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {
    
    action ingress_ptrace_log_set_meta() {
        meta.ptrace.log_flag = 1;
        meta.ptrace.flag = 1;
    } 

    table ingress_ptrace_log_marked {
        actions = {
            ingress_ptrace_log_set_meta;
        }
    }

    action ingress_ptrace_stats_set_meta() {
        meta.ptrace.log_flag = 0;
        meta.ptrace.flag = 1;
    } 

    table ingress_ptrace_stats_marked {
        actions = {
            ingress_ptrace_stats_set_meta;
        }
    }

    apply {

    }     
}

control EgressPtraceProcess(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {

    apply {

    }   
}

control EgressPtraceLogMarked(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {

    apply {

    }   
}