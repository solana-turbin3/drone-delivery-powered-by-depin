use anchor_lang::prelude::*;

#[account]
#[derive(InitSpace, Debug)]
pub struct OrderInfo {
    pub buyer: Pubkey,
    pub product: Pubkey,
    pub quantity: u32,
    pub price: u32,
    pub timestamp: i64,
    pub bump: u8,

}