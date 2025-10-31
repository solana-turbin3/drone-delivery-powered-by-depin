use anchor_lang::prelude::*;

use crate::events::*;
use crate::state::LetterboxState;
use crate::LetterboxStatus;

#[derive(Accounts)]
pub struct UpdateLetterboxStatus<'info> {
    #[account(mut)]
    pub operator: Signer<'info>,

    #[account(
        mut,
        seeds = [b"drone", operator.key().as_ref()],
        bump = drone_state.bump,
    )]
    pub drone_state: Account<'info, LetterboxState>,
    
}

impl<'info> UpdateLetterboxStatus<'info> {
    pub fn update_letterboxstatus(&mut self, status: LetterboxStatus) -> Result<()> {
    
       /*  match status {
            LetterboxStatus::Open => {
                emit!(LetterboxOpenEvent {
                    operator: self.operator.key(),
                    timestamp: clock.unix_timestamp,
                });
            }
            LetterboxStatus::Closed => {
                emit!(LetterboxClosedEvent {
                    operator: self.operator.key(),
                    timestamp: clock.unix_timestamp,
                });
            }
            }
        } */
    

    Ok(())
    }
}

