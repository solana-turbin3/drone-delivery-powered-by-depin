use anchor_lang::prelude::*;

use crate::state::*;

#[event]
pub struct OrderPlacedEvent {
    pub buyer: Pubkey,
    pub product: Pubkey,
    pub quantity: u32,
    pub price: u32,
    pub timestamp: i64,
}