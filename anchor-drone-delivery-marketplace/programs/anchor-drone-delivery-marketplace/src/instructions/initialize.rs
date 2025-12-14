use anchor_lang::prelude::*;

use crate::state::Marketplace;

#[derive(Accounts)]
#[instruction(name: String)]
pub struct Initialize<'info> {
    #[account(mut)]
    pub admin: Signer<'info>,
    #[account(
        init,
        payer = admin, 
        seeds = [b"marketplace", name.as_bytes()], //make generic
        bump, 
        space = 8 + Marketplace::INIT_SPACE
    )]
    pub marketplace: Account<'info, Marketplace>,
   
    pub system_program: Program<'info, System>,

}

impl <'info> Initialize<'info> {
    pub fn init(&mut self, name: String, fee: u32, bumps: &InitializeBumps) -> Result<()> {
        assert!(name.len() < 32);
        
        self.marketplace.set_inner(Marketplace{
            admin: self.admin.key(),
            fee,
            bump: bumps.marketplace,
            name
        });

        msg!("You created a marketplace named: {}", self.marketplace.name);
        
        Ok(())
        
    }
}