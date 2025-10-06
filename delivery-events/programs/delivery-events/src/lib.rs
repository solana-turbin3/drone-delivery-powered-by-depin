#![allow(deprecated, unexpected_cfgs)]
use anchor_lang::prelude::*;

mod instructions;
mod events;
mod state;

use instructions::*;
declare_id!("Fku2e85PeCU7Y5nYgw5WNYMBmtexKUECgUwZP73Va3GZ");

#[program]
pub mod delivery_events {
    use super::*;

    pub fn initialize(ctx: Context<Initialize>) -> Result<()> {
        ctx.accounts.initialize(&ctx.bumps)?;
        Ok(())
    }
    
    pub fn ready_to_fly(ctx: Context<ReadyToFly>) -> Result<()> {
        ctx.accounts.ready_to_fly()?;
        Ok(())
    }
}