use anchor_lang::prelude::*;

use crate::state::DroneState;

#[derive(Accounts)]

pub struct Initialize<'info> {
    #[account(mut)]
    pub operator: Signer<'info>,

    #[account(
        init,
        payer = operator,
        space = 8 + DroneState::INIT_SPACE,
        seeds = [b"drone", operator.key().as_ref()],
        bump,
    )]
    pub drone_state: Account<'info, DroneState>,
    pub system_program: Program<'info, System>,
}

impl<'info> Initialize<'info> {
    pub fn initialize(&mut self, bumps: &InitializeBumps) -> Result<()> {
        let state = &mut self.drone_state;
        state.operator = self.operator.key();
        state.last_status_update = 0;
        state.bump = bumps.drone_state;

        msg!("Drone state initialized for operator: {}", self.operator.key());

        Ok(())
    }
}