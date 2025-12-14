use anchor_lang::prelude::*;

#[account]
#[derive(InitSpace, Debug)]
pub struct Product {
    #[max_len(32)] 
    pub name: String,
    #[max_len(256)] 
    pub description: String,
    pub quantity: u32,
    pub seller: Pubkey,
    pub price: u32,
    pub bump: u8,

}