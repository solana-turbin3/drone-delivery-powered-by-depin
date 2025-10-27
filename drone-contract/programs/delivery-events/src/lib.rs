#![allow(deprecated, unexpected_cfgs)]
use anchor_lang::prelude::*;

mod instructions;
mod events;
mod state;
mod error;

use instructions::*;
use events::*;
use state::*;
use error::*;

declare_id!("9eJqD1azGBNZbtownNerLs85si5xQKr3ggJpo9ZSNJY7");

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

   /*  pub fn update_dronestatus(ctx: Context<UpdateDroneStatus>, status: state::DroneState::DroneStatus) -> Result<()> {
        ctx.accounts.update_dronestatus(status)?;
        Ok(())
    }

    pub fn update_letterboxstatus(ctx: Context<UpdateLetterboxStatus>, status: LetterboxStatus) -> Result<()> {
        ctx.accounts.update_letterboxstatus(status)?;
        Ok(())
    } */
}