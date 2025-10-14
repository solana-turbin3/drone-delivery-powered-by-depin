use anchor_lang::prelude::*;

use crate::events::ReadyToFlyEvent;
use crate::state::DroneState;

#[derive(Accounts)]
pub struct ReadyToFly<'info> {
    #[account(mut)]
    pub operator: Signer<'info>,

    #[account(
        mut,
        seeds = [b"drone", operator.key().as_ref()],
        bump = drone_state.bump,
    )]
    pub drone_state: Account<'info, DroneState>,
    
}

impl<'info> ReadyToFly<'info> {
    pub fn ready_to_fly(&mut self) -> Result<()> {
    let clock = Clock::get()?;
    let state = &mut self.drone_state;
    
    state.last_status_update = clock.unix_timestamp;

    emit!(ReadyToFlyEvent {
        operator: self.operator.key(),
        timestamp: clock.unix_timestamp,
    });

    msg!("Drone ready to fly.");

    Ok(())
    }
}
