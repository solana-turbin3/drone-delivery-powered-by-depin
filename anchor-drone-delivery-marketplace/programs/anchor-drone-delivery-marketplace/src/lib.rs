#![allow(unexpected_cfgs, deprecated)]

use anchor_lang::prelude::*;

declare_id!("9zVb1etgUuQp2PSTgYUAEC3dCrqM9rJVAQxkg6uXnsTF");

mod instructions;
mod state;
mod error;
mod constants;

pub use instructions::*;
pub use state::*;
pub use error::*;
pub use constants::*;

#[program]
pub mod anchor_drone_delivery_marketplace {
    use super::*;

    pub fn initialize(ctx: Context<Initialize>, name: String, fee: u32) -> Result<()> {
        ctx.accounts.init(name, fee, &ctx.bumps)?;
        Ok(())
    }

    //pub fn list_item()

    //pub fn purchase_item()

    //pub fn update_dronestatus()

    
}

