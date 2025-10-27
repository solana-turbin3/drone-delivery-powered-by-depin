use anchor_lang::prelude::*;

#[event]
pub struct ReadyToFlyEvent {
    pub operator: Pubkey,
    pub timestamp: i64,
}