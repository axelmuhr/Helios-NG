
..0.A:�b$Header: /users/bart/hsrc/network/telmat/TRAN/RCS/telmat_r.c,v 1.2 1992/01/14 14:26:27 bart Exp $ �
.DevOpen:�rs��(.DeviceOperate-2)�!�s�(.DeviceClose-2)�!�s�Lq�.Malloc�s�As:q�.InitSemaphore�s"��
.DeviceOperate:�`��(..21.A-2)�!��u�q:t�.Wait�v0�pD��..11.A�p`��..6.A�p`��..5.A�..4.A
..11.A:��pC��..15.A�p`��..7.A�..4.A
..15.A:��p`��..10.A�p`��..9.A�p`��..8.A�..4.A
..10.A:�vqt�.driver_Initialise�..3.A
..9.A:�vqt�.driver_Reset�..3.A
..8.A:�vqt�.driver_Analyse�..3.A
..7.A:�vqt�.driver_TestReset�..3.A
..6.A:�vqt�.driver_Boot�..3.A
..5.A:�vqt�.driver_ConditionalReset�..3.A
..4.A:�r0v�
..3.A:�q:t�.Signal��"��
..21.A:�  ��
.DeviceClose:�
`�t�p9�p:s�.Wait�q�..23.A�qs�.Close
..23.A:�p:s�.Free�@�"��
.driver_report:�Z`�~8����t0Q�st�s`?��t0Q�st�s`?��t0Q�st�s`?��t0Q�st�s`?��t0Q�st�s`?�o@�z0�u�v�w�x�y}�..26.A��"��
.driver_fatal:�P`�~8����t0Q�st�s`?��t0Q�st�s`?��t0Q�st�s`?��t0Q�st�s`?��t0Q�st�s`?�o@�~"�}�.Signal�z1�u�v�w�x�y}�..26.A��"��
.LocateServer:�`��(..39.A-2)�!��A��
..29.A:�s�..30.A�z@x�.Locate��s�s��ʊ..32.A�z�vyx�.driver_fatal�
..32.A:�t��..34.A�z�vZyx�.driver_report�..36.A�
..34.A:�@��
..36.A:�v!8x�.Delay�..29.A�
..30.A:�z�v!Yyx�.driver_report�A�@tx�.Open��u��..37.A�tx�.Result2�
�r�z�v"Qyx�.driver_fatal
..37.A:�uy�"��
..39.A:�(telmat_r.d driver, failed to locate %s  ��6telmat_r.d driver, failed to locate %s, retrying ...  ��!�� telmat_r.d driver, found %s  ��/telmat_r.d driver, failed to open %s, fault %x ��
.ExtractDriver:�`��(..46.A-2)�!��xv�.RmGetProcessorPrivate��@��
..41.A:�s!4r��..42.A�s!5r!@��0w6���..44.A�s!5r!@���"��
..44.A:�r�Ҁ..41.A�
..42.A:�xv�.RmGetProcessorId��q�twv�.driver_fatal�@�"��
..46.A:�Ptelmat_r.d, corruption detected, processor %s's driver details have been lost.
 ��
.ExtractPin:�`�tsr�.ExtractDriver��p2�"��
.driver_Initialise:�`��(..59.A-2)�!��@�w�wRzy�.driver_report�wZzy�.LocateServer�%/@y�.Malloc��u��..49.A�w<{�"�
..49.A:�uy�.Initialise_all_special�
@z�z6�t&�y�.strlen�..51.A�w]t&�y�.strcmp���..53.A�Az�..51.A
..53.A:�t&��w_zy�.driver_report
..51.A:�@��
..56.A:�t3s��..57.A�st"8�0�u�z�ry�.initialise_aux1�s�Ӏ..56.A�
..57.A:�uy�.Free�@{�"��
..59.A:�1.01 ��telmat_r.d driver version %s ��/CbMan ����silent ��&telmat_r.d driver, ignoring option %s ��
.initialise_aux1:�`��(..77.A-2)�!��!r!q�.RmGetProcessorPurpose�N$�Ċ..61.A�!�"�
..61.A:�!r!t7���..63.A�!�"�
..63.A:�!r!t!q�.ExtractDriver��s1)@$�s�!r!q�.RmCountProcessorAttributes��tJ��..65.A�	!r��!t!q�.driver_fatal
..65.A:�!r!q�.RmListProcessorAttributes�@��
..67.A:�tq��..68.A�q�0!u!q�.Is_special_processor��r��..70.A�q�0�_!t!q�.driver_report�..68.A�
..70.A:�r`���..75.A�rs�!�"��
..75.A:�q�р..67.A�
..68.A:�!s0�!s�p0�p�s�!�"��
..77.A:�9telmat_r.d driver, processor %s has too many attributes
 ��;telmat_r.d driver, not enough %s processors in this T.NODE ��
.driver_Reset:�`��(..115.A-2)�	!��}9�~1��..79.A�~2|�.RmGetProcessorState��s"@$��..79.A�@~�"�
..79.A:�
~1`�D���y|�.Malloc��v��..83.A�z0~�"�
..83.A:�#Gv#�@v�#�@��
..85.A:�~1t��..86.A�t~R�0}|�.ExtractPin�tvQ��t�Ԁ..85.A�
..86.A:�@��
..88.A:�Bw��..89.A�F#�Av�#�z1�z2�y�vu|�.PutServer�~�@~0��..91.A�u|�.ReOpen�..93.A�
..91.A:�z1��u|�.GetServer�~�@~0��..94.A�u|�.ReOpen�..93.A�
..94.A:������..89.A�Bv�#�z1�z2�y�vu|�.PutServer�~�@~0��..98.A�u|�.ReOpen�..93.A�
..98.A:�z1��u|�.GetServer�~�@~0��..100.A�u|�.ReOpen�..93.A�
..100.A:�����..89.A�
..93.A:�w�׀..88.A�
..89.A:�~0�..104.A�~0�zS}|�.driver_report�..106.A
..104.A:�����..107.A���z!S}|�.driver_report�..106.A
..107.A:�@��
..110.A:�~1t��..106.A�t~R�0|�.RmGetProcessorState��s"@$��sa/O$��st~R�0|�.RmSetProcessorState�};��..113.A�t~R�0|�.RmGetProcessorId��tvQ�0�r�z!\}|�.driver_report�
..113.A:�t�Ԁ..110.A
..106.A:�v|�.Free��"��
..115.A:�K��@KL    `telmat_r.d driver, failed to communicate with /CbMan, fault %x ��#telmat_r.d driver, CbMan error %d  ��=telmat_r.d driver, successfully reset processor /%s (Pin %d) ��
.driver_Analyse:�`��(..149.A-2)�!��}9�~1D���y|�.Malloc��u��..117.A�z0~�"�
..117.A:�#Gu#�@u�#�@��
..119.A:�~1t��..120.A�t~R�0}|�.ExtractPin�tuQ��t�Ԁ..119.A�
..120.A:�@��
..122.A:�Bw��..123.A�F#�Cu�#�z1�z2�y�uv|�.PutServer�~�@~0���..127.A�z1��v|�.GetServer�~�@~0���..127.A������..123.A�Du�#�z1�z2�y�uv|�.PutServer�~�@~0���..127.A�z1��v|�.GetServer�~�@~0���..127.A�����..123.A�
..127.A:�w�׀..122.A�
..123.A:�~0�..138.A�~0�zS}|�.driver_report�..140.A
..138.A:�����..141.A���z!S}|�.driver_report�..140.A
..141.A:�@��
..144.A:�~1t��..140.A�t~R�0|�.RmGetProcessorState��s"@$��sa/O$��st~R�0|�.RmSetProcessorState�};��..147.A�t~R�0|�.RmGetProcessorId��tuQ�0�r�z!\}|�.driver_report�
..147.A:�t�Ԁ..144.A
..140.A:�u|�.Free��"��
..149.A:�K��@KL    `telmat_r.d driver, failed to communicate with /CbMan, fault %x ��#telmat_r.d driver, CbMan error %d  ��<telmat_r.d driver, successfully analysed processor /%s (%d) ��
.driver_TestReset:�`��(..151.A-2)�!��psr�.driver_fatal��"��
..151.A:�6telmat_r.d, driver TestReset routine called illegally ��
.driver_Boot:�`��(..153.A-2)�!��psr�.driver_fatal��"��
..153.A:�6telmat_r.d, driver bootstrap routine called illegally ��
.driver_ConditionalReset:�`��(..155.A-2)�!��psr�.driver_fatal��"��
..155.A:�>telmat_r.d, driver conditional reset routine called illegally ��
..26.A:�x��..dataseg.A 1���`�s0�modnum��q�..dataseg.A��t�..157.A�..158.A
..157.A:�(..0.A-2)�!�p�
..158.A:��"��