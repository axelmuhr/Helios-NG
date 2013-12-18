
BYTE getbyte( WORD , BYTE );
int HE1000_init( WORD link , WORD target_id , WORD tram_id);
int HE1000_release( WORD link , WORD table );
int scsi_read(
	WORD link,
	WORD table,
	WORD target_id,
	WORD lun,
	WORD position,
	WORD size,
	WORD sector_size,
	BYTE *data,
	WORD *command_status);
int scsi_read_quick(
	WORD link,
	WORD table,
	WORD target_id,
	WORD lun,
	WORD position,
	WORD *command_status);
int scsi_write(
	WORD link,
	WORD table,
	WORD target_id,
	WORD lun,
	WORD position,
	WORD size,
	WORD sector_size,
	BYTE *data,
	WORD *command_status);
int scsi_write_quick(
	WORD link,
	WORD table,
	WORD target_id,
	WORD lun,
	WORD position,
	WORD *command_status);
int scsi_start_stop(
	WORD link,
	WORD table,
	WORD target_id,
	WORD lun,
	WORD start_stop,
	WORD *command_status);
int scsi_format(
	WORD link,
	WORD table,
	WORD target_id,
	WORD lun,
	WORD interleave,
	WORD *command_status);
int scsi_read_capacity(
	WORD link,
	WORD table,
	WORD target_id,
	WORD lun,
	WORD length,
	BYTE *data,
	WORD *command_status);
int scsi_request_sense(
	WORD link,
	WORD table,
	WORD target_id,
	WORD lun,
	WORD length,
	BYTE *data,
	WORD *command_status);
int scsi_test_unit_ready(
	WORD link,
	WORD table,
	WORD target_id,
	WORD lun,
	WORD *command_status);
int scsi_mode_select(
	WORD link,
	WORD table,
	WORD target_id,
	WORD lun,
	WORD block_length,
	WORD no_blocks,
	WORD *command_status);
int scsi_reassign_block(
	WORD link,
	WORD table,
	WORD target_id,
	WORD lun,
	WORD block,
	WORD *command_status);
void	recover_sync( WORD link );
void	scsi_reset( WORD link );

#define CHECK_SENSE	0x0002	/* M.E. check this value */
#define SENSE_LENGTH	0x0012
#define CAPACITY_LENGTH 0x0008
#define HE1000_SCSI_ID	0x06
