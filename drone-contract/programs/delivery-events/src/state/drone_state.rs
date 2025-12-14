use anchor_lang::prelude::*;

#[account]
#[derive(InitSpace)]
pub struct DroneState {
    pub operator: Pubkey,
    pub last_status_update: i64,
    pub bump: u8,
    pub status: DroneStatus,
}

#[derive(Debug, Clone, Copy, AnchorSerialize, AnchorDeserialize, PartialEq)]
pub enum DroneStatus {
    Available,
    ReadyToFly,
    Arrived,
    UnAvailable,
    Error
}

impl Space for DroneStatus {
    const INIT_SPACE: usize = 42;
}