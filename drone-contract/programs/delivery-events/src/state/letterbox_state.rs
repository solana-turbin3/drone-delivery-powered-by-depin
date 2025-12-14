use anchor_lang::prelude::*;

#[account]
#[derive(InitSpace)]
pub struct LetterboxState {
    pub owner: Pubkey,
    pub last_status_update: i64,
    pub bump: u8,
    pub status: LetterboxStatus,
}

#[derive(Debug, Clone, Copy, AnchorSerialize, AnchorDeserialize, PartialEq)]
pub enum LetterboxStatus {
    Open,
    Closed,
    Error
}

impl Space for LetterboxStatus {
    const INIT_SPACE: usize = 42;
}