
..0.A:�Z$Header: /users/bart/hsrc/network/TRAN/RCS/tram_ra.c,v 1.4 1992/10/28 13:03:39 bart Exp $ �
.DevOpen:�rs��(.DeviceOperate-2)�!�s�(.DeviceClose-2)�!�s�s"��
.DeviceOperate:�`��(..21.A-2)�!��u�v0�pD��..11.A�p`��..6.A�p`��..5.A�..4.A
..11.A:��pC��..15.A�p`��..7.A�..4.A
..15.A:��p`��..10.A�p`��..9.A�p`��..8.A�..4.A
..10.A:�vqt�.driver_Initialise�..3.A
..9.A:�vqt�.driver_Reset�..3.A
..8.A:�vqt�.driver_Analyse�..3.A
..7.A:�vqt�.driver_TestReset�..3.A
..6.A:�vqt�.driver_Boot�..3.A
..5.A:�vqt�.driver_ConditionalReset�..3.A
..4.A:�r0v�
..3.A:��"��
..21.A:�  ��
.DeviceClose:�@"��
.driver_report:�Z`�~8����t0Q�st�s`?��t0Q�st�s`?��t0Q�st�s`?��t0Q�st�s`?��t0Q�st�s`?�o@�z0�u�v�w�x�y}�..24.A��"��
.driver_fatal:�Z`�~8����t0Q�st�s`?��t0Q�st�s`?��t0Q�st�s`?��t0Q�st�s`?��t0Q�st�s`?�o@�z1�u�v�w�x�y}�..24.A��"��
.driver_LookupProcessor:�`�v5�v8�r2�wsu�..27.A��q�"��
.driver_rexec:�`�w8�t3�{�z�yxv�..29.A��s�"��
.driver_Initialise:�`��(..68.A-2)�!��@�x�xR{z�.driver_report�@|�{6�s3�@��
..31.A:�s3u��..32.A�
us"8�0{7���..34.A�t`�Ԁ..32.A�
..34.A:�u�Հ..31.A�
..32.A:�
@{�@s�s&�z�.strlen�@$�#��$�#����..40.A�xZs&�z�.strcmp���..38.A�Aր..40.A
..38.A:�s&��"���..41.A�s&��x\{z�.driver_report�x!V{z�.driver_report�..40.A
..41.A:�s&�{z�.driver_LookupProcessor��w��..43.A�	s&��x!_{z�.driver_report�..40.A
..43.A:�w{7�..40.A�w{�ws�
..40.A:�t��..47.A��"�
..47.A:�@��
..49.A:�s3u��..50.A�us"8�0�w{7�..56.A�w{9����..56.A�wz�.RmGetProcessorPrivate��@��
..57.A:�r!4q��..56.A�r!5q!@���p0s���..62.A�t��..63.A�p1!@$�p�..65.A�
..63.A:�p1"@$�p��
..65.A:�v�..62.A�	p1" @$�p��
..62.A:�q�р..57.A�
..56.A:�u�Հ..49.A�
..50.A:��"��
..68.A:�1.01 ��tram_ra.d driver, version %s ��reclaim ��'tram_ra.d driver, unexpected option %s ��"tram_ra.d driver, ignoring option ��6tram_ra.d driver, failed to find control processor %s ��
.driver_Reset:�`�v6�vu�.do_reset�w�w0��..70.A�@��
..72.A:�s3q��..70.A�qs"8�0�pv7�..79.A�pv9����..79.A�pu�.RmGetProcessorState�
�r"@$��rpu�.RmSetProcessorState�
..79.A:�q�р..72.A
..70.A:��"��
.do_reset:�`��(..90.A-2)�!��!p9�..82.A�!p9!p7���..81.A
..82.A:�
D�@|�"'!@�.Delay�A|�"'!@�.Delay�
@�A|�"'!@�.Delay�@|�"'!@�.Delay�	D�@|�@�"�
..81.A:�}@�.Locate��x��..86.A�}V!p�.driver_report�}!5�"�
..86.A:�@�@�@�@�����}!6��x�!p9!p�.driver_rexec��x�.Close�s�..88.A�s�}!W!p�.driver_report
..88.A:�s�"��
..90.A:�/helios/bin/tr_reset ��9tram_ra.d, failed to locate program /helios/bin/tr_reset ��H��@KL tram_ra.d warning, failed to execute program tr_reset, fault %x ��
.driver_Analyse:�`�v6�vu�.do_reset�w�w0��..92.A�@��
..94.A:�s3q��..92.A�qs"8�0�pv7�..101.A�pv9����..101.A�pu�.RmGetProcessorState�
�r"@$��rpu�.RmSetProcessorState�
..101.A:�q�р..94.A
..92.A:��"��
.driver_TestReset:�`��(..117.A-2)�!��w6�@��
..103.A:�s3q��..104.A�qs"8�0�pw7�..110.A�pw9����..110.A�pv�.RmGetProcessorState��r!@$���..110.A�t0x�"��
..110.A:�q�р..103.A�
..104.A:�w9�..113.A�w9�pv�.RmGetProcessorState��r! @$���..113.A�t1x�"�
..113.A:�@x�"��
..117.A:������
.driver_Boot:�`��(..119.A-2)�!��psr�.driver_fatal��"��
..119.A:�5tram_ra.d, driver bootstrap routine called illegally ��
.driver_ConditionalReset:�`��(..145.A-2)�!��w6�@��
..121.A:�s3q��..122.A�qs"8�0�pw7�..128.A�pw9����..128.A�pv�.RmGetProcessorState��r!@$���..128.A�t0x�"��
..128.A:�q�р..121.A�
..122.A:�w9�..131.A�w9�pv�.RmGetProcessorState��r! @$���..131.A�t1x�"�
..131.A:�wv�.do_reset�x�x0��..135.A�@��
..137.A:�s3q��..135.A�qs"8�0�pw7�..144.A�pw9����..144.A�pv�.RmGetProcessorState�
�r"@$��rpv�.RmSetProcessorState�
..144.A:�q�р..137.A
..135.A:��"��
..145.A:������
..24.A:�x��
..27.A:�t��
..29.A:�v��..dataseg.A 1���`�s0�modnum��q�..dataseg.A��t�..147.A�..148.A
..147.A:�(..0.A-2)�!�p�
..148.A:��"��