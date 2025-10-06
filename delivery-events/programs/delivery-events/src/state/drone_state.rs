use anchor_lang::prelude::*;

#[account]
#[derive(InitSpace)]
pub struct DroneState {
    pub operator: Pubkey,
    pub last_status_update: i64,
    pub bump: u8,
}