use anchor_lang::prelude::*;

#[account]
#[derive(InitSpace, Debug)]
pub struct Drone {
    pub admin: Pubkey,
    pub fee: u32, //rental fee
    pub bump: u8, 
   
    #[max_len(32)] 
    pub name: String

}

#[derive(Debug, Clone, Copy, AnchorSerialize, AnchorDeserialize, PartialEq)]
pub enum DroneStatus {
    Available,
    ReadyToFly,
    UnAvailable,
    Error
}