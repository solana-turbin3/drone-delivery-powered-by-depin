use anchor_lang::prelude::*;
use anchor_lang::error_code;


#[error_code]
pub enum ErrorCode {
    #[msg("Custom error message")]
    CustomError,
    
    #[msg("Unauthorized access.")]
    Unauthorized,

    #[msg("Drone Error")]
    DroneError,

    #[msg("Insufficient funds")]
    InsufficientFunds
}