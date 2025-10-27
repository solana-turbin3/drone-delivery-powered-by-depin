use anchor_lang::prelude::*;

use crate::events::*;
use crate::state::DroneState;

#[derive(Accounts)]
pub struct UpdateDroneStatus<'info> {
    #[account(mut)]
    pub operator: Signer<'info>,

    #[account(
        mut,
        seeds = [b"drone", operator.key().as_ref()],
        bump = drone_state.bump,
    )]
    pub drone_state: Account<'info, DroneState>,
    
}

/* impl<'info> UpdateDroneStatus<'info> {
    pub fn update_dronestatus(&mut self, status: DroneStatus) -> Result<()> {
    let clock = Clock::get()?;
    let state = &mut self.drone_state;
    
    state.last_status_update = clock.unix_timestamp;
    state.status = status;
    
    match status {
        DroneStatus::ReadyToFly => {
            emit!(ReadyToFlyEvent {
                operator: self.operator.key(),
                timestamp: clock.unix_timestamp,
            });
        }
        DroneStatus::Arrived => {
            emit!(DroneArrivedEvent {
                operator: self.operator.key(),
                timestamp: clock.unix_timestamp,
                status: status,
            });
        }
    }

    Ok(())
    }
} */
