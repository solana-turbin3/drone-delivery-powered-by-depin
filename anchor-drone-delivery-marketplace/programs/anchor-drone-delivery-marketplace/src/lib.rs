#![allow(unexpected_cfgs, deprecated)]

use anchor_lang::prelude::*;

declare_id!("9zVb1etgUuQp2PSTgYUAEC3dCrqM9rJVAQxkg6uXnsTF");

mod instructions;
mod state;
mod error;
mod constants;
mod events;

pub use instructions::*;
pub use state::*;
pub use error::*;
pub use constants::*;
pub use events::*;

#[program]
pub mod anchor_drone_delivery_marketplace {
    use super::*;

    pub fn initialize(ctx: Context<Initialize>, name: String, fee: u32) -> Result<()> {
        ctx.accounts.init(name, fee, &ctx.bumps)?;
        Ok(())
    }

    pub fn list_product(ctx: Context<ListProduct>, product_name: String, product_description: String, product_price: u32) -> Result<()> {
        ctx.accounts.list_product(product_name, product_description, product_price, &ctx.bumps)?;
        Ok(())
    }

    pub fn purchase_product(ctx: Context<PurchaseProduct>, product_name: String, quantity: u32) -> Result<()> {
        ctx.accounts.purchase_product(product_name, quantity, &ctx.bumps)?;
        Ok(())
    }

    //pub fn update_dronestatus()

    
}

