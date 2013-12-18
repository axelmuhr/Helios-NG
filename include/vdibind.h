/*------------------------------------------------------------------------
--                                                                      --
--                     G E M   V D I    L I B R A R Y			--
--                     ------------------------------                   --
--                                                                      --
--             Copyright (C) 1988, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- vdibind.h								--
--                                                                      --
--	Author:  BLV 12/7/88						--
--                                                                      --
------------------------------------------------------------------------*/

#ifdef WORD
#undef WORD
#endif
#ifdef BYTE
#undef BYTE
#endif
typedef int WORD;
typedef char BYTE;

WORD v_opnwk(WORD *, WORD *, WORD *);
WORD v_clswk(WORD);
WORD v_opnvwk(WORD *, WORD *, WORD *);
WORD v_clsvwk(WORD);
WORD v_clrwk(WORD);
WORD v_updwk(WORD);
WORD vst_load_fonts(WORD, WORD);
WORD vst_unload_fonts(WORD, WORD);
WORD vs_clip(WORD, WORD, WORD *);
WORD v_pline(WORD, WORD, WORD *);
WORD v_pmarker(WORD, WORD, WORD *);
WORD v_gtext(WORD, WORD, WORD, BYTE *);
WORD v_fillarea(WORD, WORD, WORD *);
WORD v_cellarray(WORD, WORD *, WORD, WORD, WORD, WORD, WORD *);
WORD v_contourfill(WORD, WORD, WORD, WORD);
WORD vr_recfl(WORD, WORD *);
WORD v_bar(WORD, WORD *);
WORD v_arc(WORD, WORD, WORD, WORD, WORD, WORD);
WORD v_pieslice(WORD, WORD, WORD, WORD, WORD, WORD);
WORD v_circle(WORD, WORD, WORD, WORD);
WORD v_ellarc(WORD, WORD, WORD, WORD, WORD, WORD, WORD);
WORD v_ellpie(WORD, WORD, WORD, WORD, WORD, WORD, WORD);
WORD v_ellipse(WORD, WORD, WORD, WORD, WORD);
WORD v_rbox(WORD, WORD *);
WORD v_rfbox(WORD, WORD *);
WORD v_justified(WORD, WORD, WORD, BYTE *, WORD, WORD, WORD);
WORD vswr_mode(WORD, WORD);
WORD vs_color(WORD, WORD, WORD *);
WORD vsltype(WORD, WORD);
WORD vsl_udsty(WORD, WORD);
WORD vsl_width(WORD, WORD);
WORD vsl_color(WORD, WORD);
WORD vsl_ends(WORD, WORD, WORD);
WORD vsm_type(WORD, WORD);
WORD vsm_height(WORD, WORD);
WORD vsm_color(WORD, WORD);
WORD vst_height(WORD, WORD, WORD *, WORD *, WORD *, WORD *);
WORD vst_point(WORD, WORD, WORD *, WORD *, WORD *, WORD *);
WORD vst_rotation(WORD, WORD);
WORD vst_font(WORD, WORD);
WORD vst_color(WORD, WORD);
WORD vst_effects(WORD, WORD);
WORD vst_alignment(WORD, WORD, WORD, WORD *, WORD *);
WORD vsf_interior(WORD, WORD);
WORD vsf_style(WORD, WORD);
WORD vsf_color(WORD, WORD);
WORD vsf_perimeter(WORD, WORD);
WORD vsf_udpat(WORD, WORD *, WORD);
WORD vro_cpyfm(WORD, WORD, WORD *, WORD *, WORD *);
WORD vrt_cpyfm(WORD, WORD, WORD *, WORD *, WORD *, WORD *);
WORD vr_trnfm(WORD, WORD *, WORD *);
WORD v_get_pixel(WORD, WORD, WORD, WORD *, WORD *);
WORD vsin_mode(WORD, WORD, WORD);
WORD vrq_locator(WORD, WORD, WORD, WORD *, WORD *, WORD *);
WORD vsm_locator(WORD, WORD, WORD, WORD *, WORD *, WORD *);
WORD vrq_valuator(WORD, WORD, WORD *, WORD *);
WORD vsm_valuator(WORD, WORD, WORD *, WORD *, WORD *);
WORD vrq_choice(WORD, WORD, WORD *);
WORD vsm_choice(WORD, WORD *);
WORD vrq_string(WORD, WORD, WORD, WORD *, BYTE *);
WORD vsm_string(WORD, WORD, WORD, WORD *, BYTE *);
WORD vsc_form(WORD, WORD *);
WORD vex_timv(WORD, WORD *, WORD *, WORD *);
WORD v_show_c(WORD, WORD);
WORD v_hide_c(WORD);
WORD vq_mouse(WORD, WORD *, WORD *, WORD *);
WORD vex_butv(WORD, WORD *, WORD *);
WORD vex_motv(WORD, WORD *, WORD *);
WORD vex_curv(WORD, WORD *, WORD *);
WORD vq_key_s(WORD, WORD *);
WORD vq_extnd(WORD, WORD, WORD *);
WORD vq_color(WORD, WORD, WORD, WORD *);
WORD vql_attributes(WORD, WORD *);
WORD vqm_attributes(WORD, WORD *);
WORD vqf_attributes(WORD, WORD *);
WORD vqt_attributes(WORD, WORD *);
WORD vqt_extent(WORD, BYTE *, WORD *);
WORD vqt_width(WORD, BYTE, WORD *, WORD *, WORD *);
WORD vqt_name(WORD, WORD, BYTE *);
WORD vqt_fontinfo(WORD, WORD *, WORD *, WORD *, WORD *, WORD *);
WORD vq_cellarray(WORD, WORD *, WORD, WORD, WORD *, WORD *, WORD *, WORD *);
WORD vqin_mode(WORD, WORD, WORD*);
WORD vq_chcells(WORD, WORD *, WORD *);
WORD v_exit_cur(WORD);
WORD v_enter_cur(WORD);
WORD v_curup(WORD);
WORD v_curdown(WORD);
WORD v_curright(WORD);
WORD v_curleft(WORD);
WORD v_curhome(WORD);
WORD v_eeos(WORD);
WORD v_eeol(WORD);
WORD vs_curaddress(WORD, WORD, WORD);
WORD v_curtext(WORD, BYTE *);
WORD v_rvon(WORD);
WORD v_rvoff(WORD);
WORD vq_curaddress(WORD, WORD *, WORD *);
WORD vq_tabstatus(WORD);
WORD v_hardcopy(WORD);
WORD v_dspcur(WORD, WORD, WORD);
WORD v_rmcur(WORD);
WORD v_form_adv(WORD);
WORD v_output_window(WORD, WORD *);
WORD v_clear_disp_list(WORD);
WORD v_bit_image(WORD, BYTE *, WORD, WORD, WORD, WORD, WORD, WORD *);
WORD vs_palette(WORD, WORD);
WORD vq_scan(WORD, WORD *, WORD *, WORD *, WORD *, WORD *);
WORD v_alpha_text(WORD, BYTE *);
WORD vqp_films(WORD, BYTE *);
WORD vqp_state(WORD, WORD *, WORD *, WORD *, WORD *, WORD *, WORD *);
WORD vsp_state(WORD, WORD, WORD, WORD, WORD, WORD, WORD *);
WORD vsp_save(WORD);
WORD vsp_message(WORD);
WORD vqp_error(WORD);
WORD v_meta_extents(WORD, WORD, WORD, WORD, WORD);
WORD v_write_meta(WORD, WORD, WORD *, WORD, WORD *);
WORD vm_filename(WORD, BYTE *);

