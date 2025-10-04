use anchor_lang::prelude::*;

use crate::state::DroneStatus;

#[derive(Accounts)]
pub struct UpdateDroneStatus<'info> {
   
   
    pub system_program: Program<'info, System>,

}

impl <'info> UpdateDroneStatus<'info> {
    pub fn update_dronestatus(&mut self, ) -> Result<()> {
       
        Ok(())
        
    }
}