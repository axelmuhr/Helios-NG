
..0.A:�^$Header: /users/bart/hsrc/network/meiko/TRAN/RCS/rte_c.c,v 1.2 1992/01/14 14:27:02 bart Exp $ �
.DevOpen:�rs��(.DeviceOperate-2)�!�s�(.DeviceClose-2)�!�s�Lq�.Malloc�s�As:q�.InitSemaphore�s"��
.DeviceOperate:�`��(..21.A-2)�!��u�q:t�.Wait�v0�p!B��..11.A�pa��..6.A�pa��..5.A�..4.A
..11.A:��p!A��..15.A�pa��..7.A�..4.A
..15.A:��p`��..10.A�p`��..9.A�pa��..8.A�..4.A
..10.A:�vqt�.driver_Initialise�..3.A
..9.A:�vqt�.driver_MakeLinks�..3.A
..8.A:�vqt�.driver_TestLinks�..3.A
..7.A:�vqt�.driver_ObtainProcessors�..3.A
..6.A:�vqt�.driver_MakeInitialLinks�..3.A
..5.A:�vqt�.driver_FreeProcessors�..3.A
..4.A:�r0v�
..3.A:�q:t�.Signal��"��
..21.A:�  ��
.DeviceClose:�`�t�q9�p�..23.A�ps�.Close�@q�
..23.A:�@�"��
.driver_report:�Z`�~8����t0Q�st�s`?��t0Q�st�s`?��t0Q�st�s`?��t0Q�st�s`?��t0Q�st�s`?�o@�z0�u�v�w�x�y}�..26.A��"��
.driver_fatal:�P`�~8����t0Q�st�s`?��t0Q�st�s`?��t0Q�st�s`?��t0Q�st�s`?��t0Q�st�s`?�o@�~"�}�.Signal�z1�u�v�w�x�y}�..26.A��"��
.driver_Initialise:�`��(..35.A-2)�	!��u�uRxw�.driver_report�x6�t&�w�.strlen�..29.A�t&��uYxw�.driver_report
..29.A:�u!R@w�.Locate��r��..31.A�u!Wxw�.driver_fatal
..31.A:�A�@rw�.Open��s��..33.A�rw�.Result2��q�u"Txw�.driver_fatal
..33.A:�sx�rw�.Close�@y�"��
..35.A:�1.00 ��rte_c.d driver version %s ��#rte_c.d driver, ignoring option %s ��/NetworkController ��1rte_c.d driver, failed to locate /NetworkControl ��<rte_c.d driver, failed to open /NetworkController, fault %x ��
.driver_MakeLinks:�`��(..37.A-2)�!��psr�.driver_report�p?t�"��
..37.A:�<rte_c.d, MakeLinks reconfiguration routine called illegally �����
.driver_TestLinks:�`��(..39.A-2)�!��psr�.driver_report�p?t�"��
..39.A:�<rte_c.d, TestLinks reconfiguration routine called illegally �����
.driver_FreeProcessors:�@s�"��
.driver_ObtainProcessors:�@s�"��
.make_connection:�a��(..56.A-2)�!�!�!s�.NewPort�	�!t9�!u!s�.RmGetProcessorPrivate��|!52�!v�!w!s�.RmGetProcessorPrivate��|!52�!x!�@��
..43.A:�By��..44.A�(�@�!q0�!@��B$�@�#�$@�#�z;�{�!q1�!s�.PutMsg��x@���..46.A�z!s�.ReOpen�..48.A�
..46.A:�	{�!q0�!s�.GetMsg��@x����..44.A�x!q2$�!q3����..44.A�z!s�.ReOpen�
..48.A:�y�ـ..43.A�
..44.A:�{!s�.FreePort�@x��..54.A�x@�..53.A
..54.A:�@@
..53.A:��!�"��
..56.A:���� 0  `      
�
.driver_MakeInitialLinks:�`�sЄ(.MakeInitialLinks_aux1-2)�!�s5r�.RmSearchNetwork�t�"��
.MakeInitialLinks_aux1:�`����r0Q�qr�q`?�o@�~}�.RmIsNetwork�..59.A�yЄ(.MakeInitialLinks_aux1-2)�!�~}�.RmSearchNetwork��"�
..59.A:�~}�.RmGetProcessorPurpose�N$�Ċ..61.A�@�"�
..61.A:�~}�.RmCountLinks��@��
..63.A:�zs��..64.A��s~}�.RmFollowLink��t�..70.A�t���..70.A�t}�.RmGetProcessorPurpose�N$����..70.A�~}�.RmGetProcessorUid��t}�.RmGetProcessorUid��vu���..70.A�uv���..75.A�sx���..70.A�
..75.A:�	x�t�s�~y}�.make_connection��w�..70.A�w�"��
..70.A:�s�Ӏ..63.A�
..64.A:�@�"��
..26.A:�x��..dataseg.A 1���`�s0�modnum��q�..dataseg.A��t�..81.A�..82.A
..81.A:�(..0.A-2)�!�p�
..82.A:��"��